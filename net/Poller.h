
#ifndef _poller_h__
#define _poller_h__

#include <map>
#include <vector>
#include "../base/Timestamp.h"
#include "NetworkLoop.h"
namespace mayday
{
    namespace net
    {
        class Channel;
        class NetworkLoop;
        class Poller
        {
        public:
            typedef std::vector<Channel*> ChannelList;

            Poller( NetworkLoop  *loop );
            virtual ~Poller();


            virtual Timestamp poll( int timeoutMs, ChannelList* activeChannels ) = 0;
            virtual void updateChannel( Channel* channel ) = 0;
            virtual void removeChannel( Channel* channel ) = 0;
            virtual bool hasChannel( Channel* channel ) const;
            static Poller* newDefaultPoller( NetworkLoop *loop );

            void assertInLoopThread() const
            {
                ownerLoop_->assertInLoopThread();
            }
        protected:
            typedef std::map<int, Channel*> ChannelMap;
            ChannelMap channels_;
        private:
            NetworkLoop *ownerLoop_;
        };
    }
}




#endif
