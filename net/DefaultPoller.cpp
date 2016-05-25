
#include "Poller.h"
#include "PollPoller.h"
#include "EPollPoller.h"

namespace mayday
{
    namespace net
    {
        Poller* Poller::newDefaultPoller( NetworkLoop *loop )
        {
#ifdef WIN32
            return new PollPoller( loop );
#else 
            return new EPollPoller( loop );
#endif
        }
    }
}



