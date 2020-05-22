
#include "NetworkLoop.h"
#include "Connector.h"
#include "Channel.h"
#include "SocketOps.h"


namespace mayday
{
	namespace net
	{
		Connector::Connector(NetworkLoop* loop, const std::string &name, const InetAddress& serverAddr, int timeoutSecond)
			: loop_(loop)
			, serverAddr_(serverAddr)
			, connect_(false)
			, state_(kDisconnected)
			, timeoutSecond_(timeoutSecond)
			, name_(name)
		{

		}

		Connector::~Connector()
		{
//            MDLog( "Connectord[%s] free.", name_.c_str() );
			assert(!channel_);
		}

		void Connector::start()
		{
			connect_ = true;
			loop_->runInLoop(std::bind(&Connector::startInLoop, this)); 

		}

		void Connector::startInLoop()
		{
            loop_->assertInLoopThread();
			assert(state_ == kDisconnected);
			if (connect_)
			{
				connect();
			}
		}

        //主动停止，不要给TcpClient发送通知了
		void Connector::stop()
		{
			connect_ = false;
			loop_->queueInLoop(std::bind(&Connector::stopInLoop, shared_from_this()));
		}

		void Connector::stopInLoop()
		{
            loop_->assertInLoopThread();
			if (state_ == kConnecting)
			{
				setState(kDisconnected);
				int sockfd = removeAndResetChannel();
				connectFailed(sockfd);
			}
		}

		void Connector::connect()
		{
			int sockfd = sockets::createNonblockingOrDie();
			int ret = sockets::connect(sockfd, serverAddr_.getSockAddr());
			
            int savedErrno = (ret == 0) ? 0 : NETWORK_ERROR;
			switch (savedErrno)
			{
				case 0:
#ifndef WIN32
				case EINPROGRESS:
#else 
				case WSAEWOULDBLOCK:
#endif
					connecting(sockfd);
					break;
				default:
					MDError("connect error, errorCode=%d.", savedErrno);
					assert(false);
					break;
			}
		}

		void Connector::connecting(int sockfd)
		{
			setState(kConnecting);
			assert(!channel_);
			channel_.reset(new Channel(loop_, sockfd));
			channel_->setWriteCallback(std::bind(&Connector::handleWrite, shared_from_this()));
			channel_->setErrorCallback(std::bind(&Connector::handleError, shared_from_this()));
			channel_->enableWriting();

			//开启一个链接超时检查任务
            loop_->runAfter( std::bind( &Connector::_connectTimeoutCheck, std::weak_ptr<Connector>( shared_from_this() ) ), timeoutSecond_ * 1000 );
		}

		int Connector::removeAndResetChannel()
		{
			channel_->disableAll();
			channel_->remove();
			int sockfd = channel_->fd();

			loop_->queueInLoop(std::bind(&Connector::resetChannel, shared_from_this())); 
			return sockfd;
		}

		void Connector::resetChannel()
		{
			channel_.reset();
		}

		void Connector::handleWrite()
		{
            loop_->assertInLoopThread();
			//MDLog("Connector::handleWrite:%d.", state_);

			if (state_ == kConnecting)
			{
				int sockfd = removeAndResetChannel();
				int err = sockets::getSocketError(sockfd);
				if (err)
				{
					MDLog("Connector::handleWrite - SO_ERROR = %d", err);
					setState(kDisconnected);
					connectFailed(sockfd);
				}
				else
				{
					setState(kConnected);
                    connectSuccess( sockfd );
				}
			}
			else
			{
				assert(state_ == kDisconnected);
			}
		}

		void Connector::handleError()
		{
            loop_->assertInLoopThread();
			//MDError("Connector::handleError state=%d.", state_);
			if (state_ == kConnecting)
			{
				setState(kDisconnected);
				int sockfd = removeAndResetChannel();
				int err = sockets::getSocketError(sockfd);
				//MDError("SO_ERROR = %d.", err);
				connectFailed(sockfd);
			}
		}

        void Connector::_connectTimeoutCheck( std::weak_ptr<Connector> ptr )
        {
            std::shared_ptr<Connector> sptr = ptr.lock();
            if (sptr)
            {
                sptr->connectTimeoutCheck();
            }
        }

		void Connector::connectTimeoutCheck()
		{
            loop_->assertInLoopThread();
			//连接的超时检查,和停止逻辑是一样的
			if (state_ == kConnecting)
			{
				MDLog("Connector::connectTimeoutCheck.");
				setState(kDisconnected);
				int sockfd = removeAndResetChannel();
				connectFailed(sockfd);
			}
		}


		void Connector::connectSuccess(int fd)
		{
            if (connect_)
            {
                connectCompleteCallback_( shared_from_this(), fd );
            }
            else
            {
                sockets::close( fd );
            }
		}

		void Connector::connectFailed(int fd)
		{
			sockets::close(fd);
            if (connect_)
            {
			    connectCompleteCallback_(shared_from_this(), -1);
            }
		}
	}
}