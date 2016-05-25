#ifndef _EPollPoller_h__
#define _EPollPoller_h__
#ifndef WIN32
#include "Poller.h"
#include "../base/Timestamp.h"
#include <vector>

struct epoll_event;
namespace mayday
{
    namespace net
    {
        class EPollPoller : public Poller
        {
        public:
            EPollPoller( NetworkLoop* loop );
            virtual ~EPollPoller();

            virtual Timestamp poll( int timeoutMs, ChannelList* activeChannels );
            virtual void updateChannel( Channel* channel );
            virtual void removeChannel( Channel* channel );

        private:
            static const int kInitEventListSize = 16;

            static const char* operationToString( int op );

            void fillActiveChannels( int numEvents, ChannelList* activeChannels ) const;
            void update( int operation, Channel* channel );

            typedef std::vector<struct epoll_event> EventList;

            int epollfd_;
            EventList events_;
        };
    }
}





#endif





#endif