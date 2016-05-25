
#include "Channel.h"
#include "NetworkLoop.h"
#include <sstream>
#ifndef WIN32 
#include <poll.h>
#endif
#ifdef WIN32
#define POLLRDHUP 0
//#define POLLIN 1
//#define POLLPRI 2
//#define POLLOUT 4
#endif
namespace mayday
{
    namespace net
    {
        const int Channel::kNoneEvent = 0;
#ifdef WIN32
        const int Channel::kReadEvent = POLLIN;
#else 
        const int Channel::kReadEvent = POLLIN | POLLPRI;
#endif
        const int Channel::kWriteEvent = POLLOUT;

        Channel::Channel( NetworkLoop* loop, int fd__ )
            : loop_( loop ),
            fd_( fd__ ),
            events_( 0 ),
            revents_( 0 ),
            index_( -1 ),
            addedToLoop_( false ),
            eventHandling_(false )
        {
        }

        Channel::~Channel()
        {
            assert( !addedToLoop_ );
            assert( !eventHandling_ );
        }

        void Channel::update()
        {
            addedToLoop_ = true;
            loop_->updateChannel( this );
        }

        void Channel::remove()
        {
            addedToLoop_ = false;
            loop_->removeChannel( this );
        }


        void Channel::handleEvent()
        {
            eventHandling_ = true;
#ifndef NDEBUG
            //MDLog( "channel::handleEvent revent:%d,  %s.", revents_, reventsToString().c_str() );
#endif
            //套接字关闭了，并且没有可读数据
            if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) 
            {
                if (closeCallback_) closeCallback_();
            }
            
            if (revents_ & POLLNVAL)
            {
                MDWarning( "fd = %d Channel::handle_event() POLLNVAL", fd_ );
            }

            //套接字出错了
            if (revents_ & (POLLERR | POLLNVAL))
            {
                if (errorCallback_) errorCallback_();
            }

            //数据可读 (注意windows下不支持POLLPRI和POLLRDHUP, POLLIN表示了优先级数据和普通数据的非阻塞可读)
            if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
            {
                if (readCallback_) readCallback_();
            }

            if (revents_ & POLLOUT)
            {
                if (writeCallback_) writeCallback_();
            }

            eventHandling_ = false;
        }

        std::string Channel::reventsToString() const
        {
            return eventsToString( fd_, revents_ );
        }

        std::string Channel::eventsToString() const
        {
            return eventsToString( fd_, events_ );
        }

        std::string Channel::eventsToString( int fd, int ev )
        {
            std::ostringstream oss;
            oss << fd << ": ";
            if (ev & POLLIN)
                oss << "IN ";
            if (ev & POLLPRI)
                oss << "PRI ";
            if (ev & POLLOUT)
                oss << "OUT ";
            if (ev & POLLHUP)
                oss << "HUP ";
            if (ev & POLLRDHUP)
                oss << "RDHUP ";
            if (ev & POLLERR)
                oss << "ERR ";
            if (ev & POLLNVAL)
                oss << "NVAL ";

            return oss.str().c_str();
        }
    }
}

