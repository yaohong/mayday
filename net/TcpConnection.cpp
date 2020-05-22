
#include "TcpConnection.h"
#include "NetworkLoop.h"
#include "Socket.h"
#include "Channel.h"
#include "SocketOps.h"
#include "../base/MacroDefine.h"
namespace mayday
{
    namespace net
    {

        TcpConnection::TcpConnection(
            NetworkLoop *loop,
            const std::string &name,
            int sockfd,
            const InetAddress &localAddr,
            const InetAddress &peerAddr,
            int32 recvSize,
            int32 sendSize,
			int32 cacheRecvSize,
			int32 cacheSendSize
            )
            : loop_( loop )
            , name_( name )
            , state_( kConnecting )
            , socket_( new Socket( sockfd ) )
            , channel_( new Channel( loop, sockfd ) )
            , localAddr_( localAddr )
            , peerAddr_( peerAddr )
            , context_( NULL )
            , destroyed_( false )
            , isDelaySend_(false)
            , sendDelayTime_(0)
            , maxSendCacheSize_(0)
			, recvBuffer_(cacheRecvSize)
			, sendBuffer_(cacheSendSize)

            , isStartDelayTask_(false)
            , curDelayTaskId_(0)
            , delayTaskIdAlloc_(1)
        {
            socket_->setTcpNoDelay( true );
            channel_->setReadCallback( std::bind( &TcpConnection::handleRead, this ) );
            channel_->setWriteCallback( std::bind( &TcpConnection::handleWrite, this ) );
            channel_->setCloseCallback( std::bind( &TcpConnection::handleClose, this ) );
            channel_->setErrorCallback( std::bind( &TcpConnection::handleError, this ) );

			socket_->setRecvBuffSize(recvSize);
			socket_->setSendBuffSize(sendSize);
        }

        TcpConnection::~TcpConnection()
        {
            //MDLog( "tcpConnection free." );
        }

        void TcpConnection::forceClose()
        {
            //MDLog( "fd:%d, tcpConnection:%s forceClose state_:%d.", channel_->fd(), name_.c_str(), state_ );
            if (state_ == kConnected)
            {
                setState( kDisconnecting );
                loop_->queueInLoop( std::bind( &TcpConnection::forceCloseInLoop, shared_from_this() ) );
            }

        }

        void TcpConnection::forceCloseInLoop()
        {
            loop_->assertInLoopThread();
            //MDLog( "fd:%d tcpConnection:%s forceCloseInLoop state_:%d.", channel_->fd(), name_.c_str(), state_ );
            if (state_ == kDisconnecting)
            {
                handleClose();
            }
        }

        void TcpConnection::connectEstablished()
        {
            loop_->assertInLoopThread();
            MDAssert( state_ == kConnecting );
            setState( kConnected );

            //MDLog( "fd:%d connectEstablished.", channel_->fd());
            channel_->enableReading();
            assert( connectionCallback_ );
            connectionCallback_( shared_from_this() );
        }

        void TcpConnection::connectDestroyed()
        {
            loop_->assertInLoopThread();
            //MDLog( "fd:%d tcpConnection:%s connectDestroyed state_:%d.", channel_->fd(), name_.c_str(), state_ );
            destroyed_ = true;
            if (state_ == kConnected)
            {
                setState( kDisconnected );
                channel_->disableAll();
                channel_->remove();
                connectionCallback_( shared_from_this() );
            }
        }

        //限制在单线程环境下调用
        void TcpConnection::send( const void *data, int len )
        {
            loop_->assertInLoopThread();
            if (state_ == kConnected)
            {
                if (isDelaySend_)
                {
                    //延时发送模式
                    delaySend(data, len);
                    return;
                }
                //当前没有监听可写事件(send没有失败过)并且发送缓存为0
                int32 nwrote = 0;
                int32 remaining = len;
                bool faultError = false;
                if (!channel_->isWriting())
                {
                    assert( sendBuffer_.readableBytes() == 0 );
                    nwrote = ::send( channel_->fd(), (const char *)data, len, 0 );
                    if (nwrote >= 0)
                    {
                        remaining = len - nwrote;
                        //不发送 完成通知
                        if (remaining == 0 && writeCompleteCallback_)
                        {
                            loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
                        }
                    }
                    else
                    {
                        nwrote = 0;
                        if (NETWORK_ERROR != EWOULDBLOCK)
                        {
                            MDError( "TcpConnection::send error[%d].", NETWORK_ERROR );
                            faultError = true;
                        }
                    }
                } 
                else 
                {
                    assert( sendBuffer_.readableBytes() > 0 );
                }


                assert( remaining <= len );
                if (faultError)
                {
                    forceClose();
                }
                else
                {
                    if (remaining > 0)
                    {
                        int32 writableBytes = sendBuffer_.writableBytes();
                        if (writableBytes < remaining)
                        {
                            //发送缓存满了
                            //是否应该移动缓存?
                            MDWarning( "TcpConnection::send move before writableBytes=%d, remaining=%d, readbleBytes=%d.", writableBytes, remaining, sendBuffer_.readableBytes() );
                            writableBytes = sendBuffer_.moveAfterWritableBytes();
                            if (writableBytes >= remaining)
                            {
                                sendBuffer_.move();
                                writableBytes = sendBuffer_.writableBytes();
                                MDWarning( "TcpConnection::send move before writableBytes=%d, remaining=%d, readbleBytes=%d.", writableBytes, remaining, sendBuffer_.readableBytes() );
                            }
                        }

                        if (writableBytes < remaining)
                        {
                            MDWarning( "TcpConnection::send writableBytes=%d, remaining=%d.", writableBytes, remaining );
                            forceClose();
                        }
                        else
                        {
                            sendBuffer_.append( static_cast<const char*>(data)+nwrote, remaining );
                            if (!channel_->isWriting())
                            {
                                channel_->enableWriting();
                            }
                        }
                    }
                }
            }
        }

        void TcpConnection::delaySend( const void *data, int len )
        {
            int32 writableBytes = sendBuffer_.writableBytes();
            if (writableBytes < len)
            {
                writableBytes = sendBuffer_.moveAfterWritableBytes();
                if (writableBytes >= len)
                {
                    //MDLog( "fd:%d, TcpConnection::delaySend move success.", channel_->fd());
                    sendBuffer_.move();
                }
                else 
                {
                    //缓冲区不够了
                    MDError( "fd:%d TcpConnection::delaySend writableBytes=%d, remaining=%d, readableBytes=%d.", channel_->fd(), writableBytes, len, sendBuffer_.readableBytes() );
                    forceClose();
                    return;
                }
            }
            //将发送的数据追加到缓冲区
            sendBuffer_.append( (const char *)data, len );
            if (!isStartDelayTask_)
            {
                isStartDelayTask_ = true;
                //MDLog( "fd:%d startDelayTask.", channel_->fd());
                loop_->runAfter( std::bind( &TcpConnection::_handDelaySend, std::weak_ptr<TcpConnection>( shared_from_this() ) ), sendDelayTime_ );
            }
        }

        void TcpConnection::_handDelaySend(std::weak_ptr<TcpConnection> ptr)
        {
            std::shared_ptr<TcpConnection> sptr = ptr.lock();
            if (sptr)
            {
                sptr->handDelaySend();
            }
        }

        void TcpConnection::handDelaySend()
        {
            //发送数据           
            loop_->assertInLoopThread();
            //MDLog( "fd:%d handDelaySend.", channel_->fd());
            if (state_ == kConnected)
            {
                if (channel_->isWriting())
                {
                    //监听了写事件，什么都不做
                }
                else
                {
                    int32 n = ::send( channel_->fd(), sendBuffer_.peek(), sendBuffer_.readableBytes(), 0 );
                    if (n > 0)
                    {
                        sendBuffer_.retrieve( n );
                        if (sendBuffer_.readableBytes() == 0)
                        {
                            //发送完了
                            if (writeCompleteCallback_)
                            {
                                loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
                            }
                            isStartDelayTask_ = false;
                        }
                        else 
                        {
                            //没有发送完
                            //MDLog( "fd:%d handDelaySend, enableWriting.", channel_->fd() );
                            channel_->enableWriting();
                        }
                    }
                    else
                    {
                        MDWarning( "fd:%d TcpConnection::handDelaySend send failed, code=%d", channel_->fd(), NETWORK_ERROR );
                        forceClose();
                    }
                }
            }
        }


        void TcpConnection::handleRead()
        {
            loop_->assertInLoopThread();
            if (state_ == kConnected)
            {
                int n = recvBuffer_.readFd( channel_->fd() );
                if (n > 0)
                {
                    messageCallback_( shared_from_this(), &recvBuffer_ );
                }
                else if (n == 0)
                {
                    handleClose();
                }
                else
                {
#ifdef WIN32 
                    if (NETWORK_ERROR != WSAEWOULDBLOCK)
#else
                    if (NETWORK_ERROR != EINTR && NETWORK_ERROR != EAGAIN)
#endif
                    {
                        MDError( "handleRead socket recv fail, reason=%d", NETWORK_ERROR );
                        handleClose();
                    }
                }
            }
        }

        void TcpConnection::handleWrite()
        {
            loop_->assertInLoopThread();
            MDLog( "fd:%d handleWrite.", channel_->fd());
            if (state_ == kConnected)
            {
                if (channel_->isWriting())
                {
                    int32 n = ::send( channel_->fd(), sendBuffer_.peek(), sendBuffer_.readableBytes(), 0);
                    if (n > 0)
                    {
                        sendBuffer_.retrieve( n );
                        if (sendBuffer_.readableBytes() == 0)
                        {
                            if (isDelaySend_)
                            {
                                //当前是延时发送模式
                                isStartDelayTask_ = false;
                            }
                            channel_->disableWriting();
                            if (writeCompleteCallback_)
                            {
                                loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
                            }
                        }
                    }
                    else
                    {
                        MDWarning( "fd:%d TcpConnection::handleWrite send failed, code=%d", channel_->fd(), NETWORK_ERROR );
                        forceClose();
                    }
                }
                else
                {
                    assert(false);
                }
            }
        }

        void TcpConnection::handleClose()
        {
            loop_->assertInLoopThread();
            //MDLog( "fd:%d tcpConnection:%s handleClose state_:%d, revent:%d  %s.", channel_->fd(), name_.c_str(), state_, channel_->revents_, channel_->reventsToString().c_str() );
            assert( state_ == kConnected || state_ == kDisconnecting );
            //assert(state_ == kConnected);
            setState( kDisconnected );

            //关闭所有事件的获取
            channel_->disableAll();
            channel_->remove();

            TcpConnectionPtr guardThis( shared_from_this() );
            //通知应用层发生的变化
            connectionCallback_( guardThis );
            //通知所属的TcpServer
            if (!destroyed_)
            {
                closeCallback_( guardThis );
            }
        }

        void TcpConnection::handleError()
        {
            loop_->assertInLoopThread();
            //MDLog( "fd:%d tcpConnection:%s handleError state_:%d, revent:%d.", channel_->fd(), name_.c_str(), state_, channel_->revents_ );
            int err = sockets::getSocketError( channel_->fd() );
            //MDLog( "fd:%d tcpConnection:%s handleError state_:%d, errorCode:%d.", channel_->fd(), name_.c_str(), state_, err );
            if (state_ == kConnected)
            {
                handleClose();
            }
        }
    }
}