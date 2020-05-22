#ifndef _NetworkLoop_h__
#define _NetworkLoop_h__

#include "../base/Define.h"
#include "../base/Timestamp.h"
#include "../base/Mutex.h"
#include "../base/TimerManager.h"
namespace mayday
{
    namespace net
    {
        class Poller;
        class Channel;
        class NetworkLoop
        {
        public:
            NetworkLoop(int maxSTimeSlot = 1000, int maxMSTimeSlot = 100);
            ~NetworkLoop();

            typedef std::function<void( )> Functor;
        public:
            void loop();
            void quit();

            void wakeup();
            void updateChannel( Channel* channel );
            void removeChannel( Channel* channel );
            bool hasChannel( Channel* channel );

            void assertInLoopThread()
            {
                if (!isInLoopThread())
                {
                    abortNotInLoopThread();
                }
            }
            bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
            bool eventHandling() const { return eventHandling_; }

            void runInLoop( const Functor& cb );
            void queueInLoop( const Functor& cb );


            uint64 runAfter( const Functor &cb, uint32 delayTime );
            uint64 runEvery( const Functor  &cb, uint32 milseconds);

            void setTimeoutCallback( const Functor& cb ) {
                timeoutFunctor_ = cb;
            };

            static NetworkLoop* getEventLoopOfCurrentThread();
        private:
            void abortNotInLoopThread();
            //处理wakeupFd_的事件
            void handleRead();
            void doPendingFunctors( );
            void doTimeTaskFunctors( Timestamp& pollReturnTime );
        private:
            bool looping_;
            bool quit_;
            bool eventHandling_;
            bool callingPendingFunctors_;
            const pid_t threadId_;
            Timestamp prevPollReturnTime_;

            std::unique_ptr<Poller> poller_;
            std::unique_ptr<TimerManager> timerManager_;
            int wakeupFd_;
#ifdef WIN32 
            int wakeSendFd_;
            struct sockaddr_in wakeupFdLocakAddr_;
#endif
            std::unique_ptr<Channel> wakeupChannel_;

            typedef std::vector<Channel*> ChannelList;
            ChannelList activeChannels_;
            Channel* currentActiveChannel_;

            MutexLock mutex_;
            std::vector<Functor> pendingFunctors_;

            Functor timeoutFunctor_;
            
        };
    }
}



#endif