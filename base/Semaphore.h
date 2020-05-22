#ifndef _Semaphore_h__
#define _Semaphore_h__
#include "Define.h"
namespace mayday
{
    static const size_t maxSemValue = 8;
    class Semaphore
    {
    public:
        Semaphore()
        {
#ifdef WIN32 
            sem_ = ::CreateSemaphore( NULL, 0, maxSemValue, NULL );
#else 
            sem_init( &sem_, 0, 0 );
#endif
        }

        ~Semaphore()
        {
#ifdef WIN32 
            ::CloseHandle(sem_);
#else 
            sem_destroy(&sem_);
#endif
        }

    public:
        void Release()
        {
#ifdef WIN32 
            ::ReleaseSemaphore( sem_, 1, NULL );
#else 
            sem_post(&sem_);
#endif
        }

        void Wait()
        {
#ifdef WIN32 
            ::WaitForSingleObject( sem_, INFINITE );
#else 
            while (-1 == sem_wait( &sem_ ) && errno == EINTR);
#endif
        }
    private:
#ifndef WIN32
        sem_t sem_;
#else 
        HANDLE sem_;
#endif
    };
}








#endif