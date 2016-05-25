
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
            int32 sendSize
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
            , recvBuffer_( recvSize )
            , sendBuffer_( sendSize )
        {
            channel_->setReadCallback( std::bind( &TcpConnection::handleRead, this ) );
            channel_->setWriteCallback( std::bind( &TcpConnection::handleWrite, this ) );
            channel_->setCloseCallback( std::bind( &TcpConnection::handleClose, this ) );
            channel_->setErrorCallback( std::bind( &TcpConnection::handleError, this ) );
        }

        TcpConnection::~TcpConnection()
        {
            MDLog( "tcpConnection free." );
        }

        void TcpConnection::forceClose()
        {
            MDLog( "tcpConnection:%s forceClose state_:%d.", name_.c_str(), state_ );
            if (state_ == kConnected)
            {
                setState( kDisconnecting );
                loop_->queueInLoop( std::bind( &TcpConnection::forceCloseInLoop, shared_from_this() ) );
            }

        }

        void TcpConnection::forceCloseInLoop()
        {
            loop_->assertInLoopThread();
            MDLog( "tcpConnection:%s forceCloseInLoop state_:%d.", name_.c_str(), state_ );
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

            channel_->enableReading();
            assert( connectionCallback_ );
            connectionCallback_( shared_from_this() );
        }

        void TcpConnection::connectDestroyed()
        {
            loop_->assertInLoopThread();
            MDLog( "tcpConnection:%s connectDestroyed state_:%d.", name_.c_str(), state_ );
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
                            sendBuffer_.move();
                            writableBytes = sendBuffer_.writableBytes();
                            MDWarning( "TcpConnection::send move before writableBytes=%d, remaining=%d, readbleBytes=%d.", writableBytes, remaining, sendBuffer_.readableBytes() );
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
            MDLog( "handleWrite." );
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
                            channel_->disableWriting();
                            if (writeCompleteCallback_)
                            {
                                loop_->queueInLoop( std::bind( writeCompleteCallback_, shared_from_this() ) );
                            }
                        }
                    }
                    else
                    {
                        MDWarning("TcpConnection::handleWrite send failed, code=%d", NETWORK_ERROR);
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
            MDLog( "tcpConnection:%s handleClose state_:%d, revent:%d  %s.", name_.c_str(), state_, channel_->revents_, channel_->reventsToString().c_str() );
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
            MDLog( "tcpConnection:%s handleError state_:%d, revent:%d.", name_.c_str(), state_, channel_->revents_ );
            int err = sockets::getSocketError( channel_->fd() );
            MDLog( "tcpConnection:%s handleError state_:%d, errorCode:%d.", name_.c_str(), state_, err );
            if (state_ == kConnected)
            {
                handleClose();
            }
        }
    }
}