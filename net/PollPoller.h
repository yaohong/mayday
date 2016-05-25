#ifndef _poll_poller_h__
#define _poll_poller_h__

#include "Poller.h"
#include "../base/Timestamp.h"
#include <vector>

struct pollfd;
namespace mayday
{
    namespace net 
    {
        class Channel;
        class PollPoller : public Poller
        {
        public:
            PollPoller( NetworkLoop *loop );
            virtual ~PollPoller();

            Timestamp poll(int timeoutMs, ChannelList* activeChannels);
            virtual void updateChannel(Channel* channel);
            virtual void removeChannel(Channel* channel);

        private:
            void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
            typedef std::vector<struct pollfd> PollFdList;
            PollFdList pollfds_;
        }; 
    }
}


#endif  // _poll_poller_h__
