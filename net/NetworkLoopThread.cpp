#include "NetworkLoopThread.h"
#include "NetworkLoop.h"


namespace mayday
{
    namespace net
    {
        NetworkLoopThread::NetworkLoopThread( const ThreadInitCallback& cb )
            : loop_(NULL)
            , exiting_(false)
            , thread_( std::bind( &NetworkLoopThread::threadFunc, this ) )
            , sem_()
            , callback_(cb)
        {

        }

        NetworkLoopThread::~NetworkLoopThread()
        {
            exiting_ = true;
            if (loop_ != NULL)
            {
                loop_->quit();
                thread_.join();
            }
        }

        NetworkLoop* NetworkLoopThread::startLoop()
        {
            assert( !thread_.started() );
            thread_.start();
            sem_.Wait();
            assert( loop_ );
            return loop_;
        }

        void NetworkLoopThread::threadFunc()
        {
            NetworkLoop loop;

            if (callback_)
            {
                callback_( &loop );
            }

            {
                loop_ = &loop;
                sem_.Release();
            }

            loop.loop();
            loop_ = NULL;
        }
    }
}

