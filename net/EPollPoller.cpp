#include "EPollPoller.h"


#ifndef WIN32
#include "../base/Define.h"
#include "Channel.h"
#include <sys/epoll.h>
namespace
{
    const int kNew = -1;
    const int kAdded = 1;
    const int kDeleted = 2;
}

namespace mayday
{
    namespace net
    {
        EPollPoller::EPollPoller( NetworkLoop* loop )
            : Poller(loop)
            , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
            , events_(kInitEventListSize)

        {
            if (epollfd_ < 0)
            {
                MDFatal("epoll_create1 faild, errno=%d.", errno);
            }

            MDLog("epollFd_:%d", epollfd_);
        }
        EPollPoller::~EPollPoller()
        {
            ::close(epollfd_);
        }

        Timestamp EPollPoller::poll( int timeoutMs, ChannelList* activeChannels )
        {
            int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
            int savedErrno = errno;
            Timestamp now(Timestamp::now());

            if (numEvents > 0)
            {
                fillActiveChannels(numEvents, activeChannels);
                if (implicit_cast<size_t>(numEvents) == events_.size())
                {
                    events_.resize(events_.size()*2);
                    MDLog("epoll resize:%d.", events_.size());
                }
            } 
            else if (numEvents == 0)
            {

            } else 
            {
                if (savedErrno != EINTR)
                {
                    errno = savedErrno;
                    MDFatal("epoll_wait failed errno:%d.", savedErrno);
                }
            }

            return now;
        }

        void EPollPoller::fillActiveChannels( int numEvents, ChannelList* activeChannels ) const
        {
            assert(implicit_cast<size_t>(numEvents) <= events_.size());
            for (int i = 0; i < numEvents; ++i)
            {
                Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
                int fd = channel->fd();
                ChannelMap::const_iterator it = channels_.find(fd);
                assert(it != channels_.end());
                assert(it->second == channel);
#endif
                channel->set_revents(events_[i].events);
                activeChannels->push_back(channel);
            }
        }
        
        void EPollPoller::updateChannel( Channel* channel )
        {
            const int index = channel->index();
            if (index == kNew || index == kDeleted)
            {
                //新的channel,或者之前通过epoll_ctl移除的channel
                int fd = channel->fd();
                if (index == kNew)
                {
                    assert(channels_.find(fd) == channels_.end());
                    channels_[fd] = channel;
                }
                else // index == kDeleted
                {
                    assert( channels_.find( fd ) != channels_.end() );
                    assert( channels_[fd] == channel );
                }

                channel->set_index( kAdded );
                update( EPOLL_CTL_ADD, channel );
            }
            else
            {
                // update existing one with EPOLL_CTL_MOD/DEL
                int fd = channel->fd();(void)fd;
                assert( channels_.find( fd ) != channels_.end() );
                assert( channels_[fd] == channel );
                assert( index == kAdded );
                if (channel->isNoneEvent())
                {
                    //不监听事件了,从队列里移除
                    update( EPOLL_CTL_DEL, channel );
                    channel->set_index( kDeleted );
                }
                else
                {   
                    //更新监听的事件
                    update( EPOLL_CTL_MOD, channel );
                }
            }
        }

        void EPollPoller::removeChannel( Channel* channel )
        {
            int fd = channel->fd();
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
            assert(channel->isNoneEvent());
            int index = channel->index();
            assert( index == kAdded || index == kDeleted );
            size_t n = channels_.erase( fd );
            (void)n;
            assert( n == 1 );

            if (index == kAdded)
            {
                update( EPOLL_CTL_DEL, channel );
            }
            channel->set_index( kNew );
        }

        void EPollPoller::update( int operation, Channel* channel )
        {
            struct epoll_event event;
            bzero(&event, sizeof event);
            event.events = channel->events();
            event.data.ptr = channel;
            int fd = channel->fd();
#ifndef NDEBUG
            //MDLog("epoll_ctl op:%s, fd=%d, event = %d,  {%s}.", operationToString( operation ), fd, channel->events(), channel->eventsToString().c_str());
#endif

            if (::epoll_ctl( epollfd_, operation, fd, &event ) < 0)
            {
                if (operation == EPOLL_CTL_DEL)
                {
                    MDError("epoll_ctl op:%s fd=%d.", operationToString( operation ), fd);
                }
                else
                {
                    MDFatal("epoll_ctl op:%s fd=%d.", operationToString( operation ), fd);
                }
            }
        }

        const char* EPollPoller::operationToString( int op )
        {
            switch (op)
            {
                case EPOLL_CTL_ADD:
                    return "ADD";
                case EPOLL_CTL_DEL:
                    return "DEL";
                case EPOLL_CTL_MOD:
                    return "MOD";
                default:
                    return "Unknown Operation";
            }
        }
    }
}

#endif