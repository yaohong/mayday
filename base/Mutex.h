#ifndef _Mutex_h__
#define _Mutex_h__


#include "Define.h"
#include "CurrentThread.h"
namespace mayday
{
    class MutexLock
    {
    public:
        MutexLock()
        {
#ifndef WIN32 
            pthread_mutex_init(&mutex_, NULL);
#else 
            mutex_ = ::CreateMutex( NULL, FALSE, NULL );
#endif
        }

        ~MutexLock()
        {
#ifndef WIN32 
            pthread_mutex_destroy( &mutex_ );
#else 
            ::CloseHandle( mutex_ );
#endif
        }

        bool isLockedByThisThread() const
        {
            return holder_ == CurrentThread::tid();
        }

        void assertLocked() const
        {
            assert( isLockedByThisThread() );
        }

        void lock()
        {
#ifndef WIN32
            pthread_mutex_lock( &mutex_ );
#else 
            WaitForSingleObject( mutex_, INFINITE );
#endif
            holder_ = CurrentThread::tid();
        }

        void unlock()
        {
#ifndef WIN32
            pthread_mutex_unlock( &mutex_ );
#else
            ::ReleaseMutex( mutex_ );
            holder_ = 0;
#endif 
        }

    private:
#ifndef WIN32
        pthread_mutex_t mutex_;
#else 
        HANDLE mutex_;
#endif

        pid_t holder_;
    };


    class MutexLockGuard
    {
    public:
        explicit MutexLockGuard( MutexLock& mutex )
            : mutex_( mutex )
        {
            mutex_.lock();
        }

        ~MutexLockGuard()
        {
            mutex_.unlock();
        }

    private:
        MutexLock& mutex_;
    };
}








#endif