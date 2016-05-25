#ifndef _Thread_h__
#define _Thread_h__
#include "Define.h"
#include "Atomic.h"

namespace mayday
{
    class Thread
    {
    public:
        typedef std::function<void()> ThreadFunc;
        Thread( const ThreadFunc & );
        ~Thread();
    public:
        void start();
        int join();

        bool started() const { return started_; }
        uint32 tid() const { return pthreadId_; }

        static int numCreated() { return numCreated_.get(); }
    private:
        bool       started_;
        bool       joined_;
        pthread_t  pthreadId_;
        ThreadFunc func_;
        static AtomicInt32 numCreated_;
#ifdef WIN32 
        HANDLE handle_;
#endif
    };
}











#endif