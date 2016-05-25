
#include "PollPoller.h"
#include "Channel.h"
#ifndef WIN32 
#include <assert.h>
#include <errno.h>
#include <poll.h>
#endif

namespace mayday
{
    namespace net 
    {
        PollPoller::PollPoller(NetworkLoop *loop)
            : Poller(loop)
        {
        }

        PollPoller::~PollPoller()
        {
        }

        Timestamp PollPoller::poll(int timeoutMs, ChannelList* activeChannels)
        {
            // XXX pollfds_ shouldn't change
#ifndef WIN32
            int numEvents = ::poll(&*pollfds_.begin(), pollfds_.size(), timeoutMs);
            int savedErrno = errno;
#else 
            int numEvents = ::WSAPoll( &*pollfds_.begin( ), pollfds_.size( ), timeoutMs );
#endif
            Timestamp now( Timestamp::now() );
            if (numEvents > 0)
            {
                fillActiveChannels(numEvents, activeChannels);
            }
            else if (numEvents == 0)
            {
                /*ZBLog( "poll numEvents=0" );*/
            }
            else
            {
#ifdef WIN32 
                MDError("WSAPoll Error numEvents=%d, errno=%d", numEvents, NETWORK_ERROR);
#endif
#ifndef WIN32
                if (savedErrno != EINTR)
                {
                    MDError( "poll Error numEvents=%d, errno=%d", numEvents, NETWORK_ERROR );
                    errno = savedErrno;
                    MDAssert( false );
                }
#endif
                MDAssert( false );
            }

            return now;
        }

        void PollPoller::fillActiveChannels(int numEvents,
            ChannelList* activeChannels) const
        {
            for (PollFdList::const_iterator pfd = pollfds_.begin();
                pfd != pollfds_.end(); ++pfd)
            {
                if (pfd->revents > 0)
                {
                    ChannelMap::const_iterator ch = channels_.find(pfd->fd);
                    assert(ch != channels_.end());
                    Channel* channel = ch->second;
                    assert(channel->fd() == pfd->fd);
                    channel->set_revents(pfd->revents);
                    // pfd->revents = 0;
                    activeChannels->push_back(channel);
                }
            }
        }

        void PollPoller::updateChannel(Channel* channel)
        {
#ifndef NDEBUG
            MDLog( "updateChannel fd:%d, event[%d], [%s].", channel->fd(), channel->events(), channel->eventsToString().c_str() );
#endif
            if (channel->index() < 0)
            {
                assert(channels_.find(channel->fd()) == channels_.end());
                struct pollfd pfd;
                pfd.fd = channel->fd();
                pfd.events = static_cast<short>(channel->events());
                pfd.revents = 0;
                pollfds_.push_back(pfd);
                int idx = static_cast<int>(pollfds_.size())-1;
                channel->set_index(idx);
                channels_[pfd.fd] = channel;
            }
            else
            {
                // update existing one
                assert(channels_.find(channel->fd()) != channels_.end());
                assert(channels_[channel->fd()] == channel);
                int idx = channel->index();
                assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
                struct pollfd& pfd = pollfds_[idx];
                assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd()-1);
                pfd.events = static_cast<short>(channel->events());
                pfd.revents = 0;
                if (channel->isNoneEvent())
                {
                    // ignore this pollfd
                    pfd.fd = -channel->fd()-1;
                }
            }
        }

        void PollPoller::removeChannel(Channel* channel)
        {
            //ZBLog( "removeChannel fd:%d", channel->fd() );
            assert(channels_.find(channel->fd()) != channels_.end());
            assert(channels_[channel->fd()] == channel);
            assert(channel->isNoneEvent());
            int idx = channel->index();
            assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
            const struct pollfd& pfd = pollfds_[idx]; (void)pfd;
            assert(pfd.fd == -channel->fd()-1 && pfd.events == channel->events());
            size_t n = channels_.erase(channel->fd());
            assert(n == 1); (void)n;
            if (implicit_cast<size_t>(idx) == pollfds_.size()-1)
            {
                pollfds_.pop_back();
            }
            else
            {
                int channelAtEnd = pollfds_.back().fd;
                iter_swap(pollfds_.begin()+idx, pollfds_.end()-1);
                if (channelAtEnd < 0)
                {
                    channelAtEnd = -channelAtEnd-1;
                }
                channels_[channelAtEnd]->set_index(idx);
                pollfds_.pop_back();
            }
        }
    }
}