#include "Thread.h"
#include "CurrentThread.h"


namespace mayday
{

    namespace detail
    {
#ifndef WIN32 
        pid_t gettid()
        {
            return static_cast<pid_t>(pthread_self());
        }
#endif
        struct ThreadData
        {
            typedef mayday::Thread::ThreadFunc ThreadFunc;
            ThreadFunc func_;

            ThreadData( const ThreadFunc &func )
                : func_( func )
            {
            }

            void runInThread()
            {
                func_();
            }
        };

#ifdef WIN32
        uint32 __stdcall  startThread( void *obj )
#else 
        void *startThread( void *obj )
#endif
        {
            ThreadData* data = static_cast<ThreadData*>(obj);
            data->runInThread();
            delete data;
            return NULL;
        }
    }

    namespace CurrentThread
    {
#ifndef WIN32 
        __thread uint32 t_cachedTid = 0;

        void cacheTid()
        {
            if (t_cachedTid == 0)
            {
                t_cachedTid = detail::gettid();
            }
        }
#endif
    }



    Thread::Thread( const ThreadFunc& func )
        : started_( false ),
        joined_( false ),
        pthreadId_( 0 ),
        func_( func )
    {
#ifdef WIN32 
        handle_ = NULL;
#endif
    }

    Thread::~Thread()
    {
#ifndef WIN32
        if (started_ && !joined_)
        {
            pthread_detach( pthreadId_ );
        }
#endif
    }

    void Thread::start()
    {
        assert( !started_ );
        started_ = true;
        // FIXME: move(func_)
        detail::ThreadData* data = new detail::ThreadData( func_ );
#ifndef WIN32
        pthread_create( &pthreadId_, NULL, &detail::startThread, data );
#else 
        handle_ = (HANDLE)::_beginthreadex( NULL, 0, &detail::startThread, data, 0, &pthreadId_ );
#endif
    }

    int Thread::join()
    {
        assert( started_ );
        assert( !joined_ );
        joined_ = true;
#ifndef WIN32
        return pthread_join( pthreadId_, NULL );
#else 
        return ::WaitForSingleObject( handle_, INFINITE );
#endif
    }
}
