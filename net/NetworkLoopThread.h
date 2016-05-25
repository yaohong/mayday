#ifndef _NetworkLoopThread_h__
#define _NetworkLoopThread_h__

#include "../base/Mutex.h"
#include "../base/Semaphore.h"
#include "../base/Thread.h"

namespace mayday
{
    namespace net
    {
        class NetworkLoop;
        class NetworkLoopThread
        {
        public:
            typedef std::function<void( NetworkLoop* )> ThreadInitCallback;
            NetworkLoopThread( const ThreadInitCallback& cb );
            ~NetworkLoopThread();

            NetworkLoop *startLoop();
        private:
            void threadFunc();
            NetworkLoop* loop_;
            bool exiting_;
            Thread thread_;
            Semaphore sem_;
            ThreadInitCallback callback_;
        };
    }
}










#endif