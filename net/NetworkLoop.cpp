#include "../base/Define.h"
#include "../base/CurrentThread.h"
#include "NetworkLoop.h"
#include "Poller.h"
#include "Channel.h"
#include "SocketOps.h"
#include "InetAddress.h"

namespace
{
    const int kPollTimeMs = 5;
    const int kMSSize = 10;         //以10毫秒为粒度
#ifndef WIN32 
    int createEventfd()
    {
        int evtfd = ::eventfd( 0, EFD_NONBLOCK | EFD_CLOEXEC );
        if (evtfd < 0)
        {
            MDFatal( "Failed in eventfd");
            abort();
        }
        return evtfd;
    }

    class IgnoreSigPipe
    {
    public:
        IgnoreSigPipe()
        {
            ::signal( SIGPIPE, SIG_IGN );
        }
    };

    IgnoreSigPipe initObj;
#else 
    //创建一个UDP套接字
    int createEventfd()
    {
        int s = ::socket( AF_INET, SOCK_DGRAM, 0 );
        if (s < 0)
        {
            MDFatal("createEventfd failed.");
        }
        mayday::net::InetAddress listenAddr( "127.0.0.1", 0 );
        mayday::net::sockets::bind( s, listenAddr.getSockAddr() );

        return s;
    }

    class SocketInit
    {
    public:
        SocketInit()
        {
            WORD wVer = MAKEWORD( 2, 2 );
            WSADATA wsaData;
            WSAStartup( wVer, &wsaData );
        }
    };

    SocketInit initObj;
#endif
}
namespace mayday
{
    namespace net
    {

        NetworkLoop* NetworkLoop::getEventLoopOfCurrentThread()
        {
            return NULL;
        }

        NetworkLoop::NetworkLoop( int maxSTimeSlot, int maxMSTimeSlot )
            : looping_(false)
            , quit_( false )
            , eventHandling_(false)
            , callingPendingFunctors_(false)
            , threadId_(CurrentThread::tid())
            , poller_( Poller::newDefaultPoller( this ) )
            , timerManager_(new TimerManager())
            , wakeupFd_( createEventfd ())
            , wakeupChannel_( new Channel( this, wakeupFd_ ) )
            , currentActiveChannel_( NULL )
        {
#ifdef WIN32
            //需要获取wakeupfd的本地地址
            wakeupFdLocakAddr_ = sockets::getLocalAddr( wakeupFd_ );
            wakeSendFd_ = ::socket( AF_INET, SOCK_DGRAM, 0 );
            if (wakeSendFd_ < 0)
            {
                MDFatal( "createEventfd failed." );
                abort();
            }
#endif
            wakeupChannel_->setReadCallback(std::bind( &NetworkLoop::handleRead, this ) );
            wakeupChannel_->enableReading();
        }

        NetworkLoop::~NetworkLoop()
        {
            wakeupChannel_->disableAll();
            wakeupChannel_->remove();
            sockets::close( wakeupFd_ );
#ifdef WIN32 
            sockets::close( wakeSendFd_ );
#endif
        }
        void NetworkLoop::runInLoop( const Functor& cb )
        {
            if (isInLoopThread())
            {
                cb();
            }
            else
            {
                queueInLoop( cb );
            }
        }

        void NetworkLoop::queueInLoop( const Functor& cb )
        {
            {
                MutexLockGuard lock(mutex_);
                pendingFunctors_.push_back( cb );
            }

            //不是当前线程，或者是当前线程但是已经开始执行延时任务了
            if (!isInLoopThread() || callingPendingFunctors_)
            {
                wakeup();
            }
            
        }

        uint64 NetworkLoop::runAfter( const Functor &cb, uint32 delayTime )
        {
            return timerManager_->runAfter( delayTime, cb );
        }

        uint64 NetworkLoop::runEvery( const Functor  &cb, uint32 milseconds )
        {
            return timerManager_->runEvery( milseconds , cb);
        }


        void NetworkLoop::loop()
        {
            assert( !looping_ );
            looping_ = true;
            quit_ = false;
            prevPollReturnTime_ = Timestamp::now();
            while (!quit_)
            {
                activeChannels_.clear();
                Timestamp pollReturnTime = poller_->poll( kPollTimeMs, &activeChannels_ );

                eventHandling_ = true;
                for (ChannelList::iterator it = activeChannels_.begin(); it != activeChannels_.end(); ++it)
                {
                    currentActiveChannel_ = *it;
                    currentActiveChannel_->handleEvent();
                }

                currentActiveChannel_ = NULL;
                eventHandling_ = false;
                timerManager_->detectTimer( pollReturnTime );
                doPendingFunctors();
                if (timeoutFunctor_)
                {
                    timeoutFunctor_();
                }
            }

            looping_ = false;
        }

        void NetworkLoop::quit()
        {
            quit_ = true;
        }

        void NetworkLoop::updateChannel( Channel* channel )
        {
            assert( channel->ownerLoop() == this );
            assertInLoopThread();
            poller_->updateChannel( channel );
        }

        void NetworkLoop::removeChannel( Channel* channel )
        {
            assertInLoopThread();
            assert( channel->ownerLoop() == this );
            if (eventHandling_)
            {
                assert( currentActiveChannel_ == channel ||
                    std::find( activeChannels_.begin(), activeChannels_.end(), channel ) == activeChannels_.end() );
            }

            poller_->removeChannel( channel );
        }

        bool NetworkLoop::hasChannel( Channel* channel )
        {
            assert( channel->ownerLoop() == this );
            assertInLoopThread();
            return poller_->hasChannel( channel );
        }

        void NetworkLoop::abortNotInLoopThread()
        {
            MDFatal( "NetworkLoop::abortNotInLoopThread, was created in threadId_:%d, current thread id:%d", threadId_, CurrentThread::tid() );
        }

        void NetworkLoop::wakeup()
        {
#ifdef WIN32 
            int64 one = 1;
            int n = ::sendto(wakeSendFd_, (const char *)&one, sizeof one, 0, (const sockaddr *)&wakeupFdLocakAddr_, sizeof wakeupFdLocakAddr_);
            if (n != sizeof one)
            {
                MDError("NetworkLoop::wakeup() writes:%d bytes instead of 8", n);
            }
#else 
            uint64_t one = 1;
            ssize_t n = ::write(wakeupFd_, &one, sizeof one);
            if (n != sizeof one)
            {
                MDError("NetworkLoop::wakeup() writes:%d bytes instead of 8", n);
            }
#endif
        }

        void NetworkLoop::handleRead()
        {
#ifdef WIN32 
            int64 one = 0;
            sockaddr_in from_addr;
            int from_len = sizeof from_addr;
            int n = ::recvfrom( wakeupFd_, (char *)&one, sizeof one, 0, (sockaddr *)&from_addr, &from_len );
            if (n != sizeof one)
            {
                MDError( "NetworkLoop::handleRead() reads:%d bytes instead of 8", n );
            }
            
#else 
            uint64_t one = 0;
            ssize_t n = ::read( wakeupFd_, &one, sizeof one );
            if (n != sizeof one)
            {
                MDError("NetworkLoop::handleRead() reads:%d bytes instead of 8", n);
            }
#endif
            MDLog("NetworkLoop::handleRead.");
        }

        void NetworkLoop::doPendingFunctors()
        {
            std::vector<Functor> functors;
            callingPendingFunctors_ = true;

            {
                MutexLockGuard lock( mutex_ );
                functors.swap( pendingFunctors_ );
            }
            

            for (size_t i = 0; i < functors.size(); ++i)
            {
                functors[i]();
            }
            callingPendingFunctors_ = false;
        }
    }
}



