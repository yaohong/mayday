#ifndef _NetworkLoop_h__
#define _NetworkLoop_h__

#include "../base/Define.h"
#include "../base/Timestamp.h"
#include "../base/Mutex.h"
namespace mayday
{
    namespace net
    {
        class Poller;
        class Channel;
        class NetworkLoop
        {
        public:
            NetworkLoop(int maxTimeSlot = 1024);
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




            void addDelayTask( const Functor &cb, int delayTime );
            void setTimeoutCallback( const Functor& cb ) {
                timeoutFunctor_ = cb;
            };

            static NetworkLoop* getEventLoopOfCurrentThread();
        private:
            void abortNotInLoopThread();
            //����wakeupFd_���¼�
            void handleRead();
            void doPendingFunctors( );
            void doTimeTaskFunctors(Timestamp& pollReturnTime );
        private:
            bool looping_;
            bool quit_;
            bool eventHandling_;
            bool callingPendingFunctors_;
            const pid_t threadId_;
            Timestamp prevPollReturnTime_;
            std::unique_ptr<Poller> poller_;

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



            std::vector<std::list<Functor> > timeTasks_;
            int curTick_;
            int maxTimeSlot_;


            Functor timeoutFunctor_;
            
        };
    }
}



#endif