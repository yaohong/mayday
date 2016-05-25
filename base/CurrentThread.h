#ifndef _CurrentThread_h__
#define _CurrentThread_h__
#include "Define.h"
namespace mayday
{
    namespace CurrentThread
    {
#ifndef WIN32 
        extern __thread uint32 t_cachedTid;
        void cacheTid();
#endif
        inline pid_t tid()
        {
#ifdef WIN32 
            return ::GetCurrentThreadId();
#else 
            cacheTid();
            return t_cachedTid;
#endif
        }
    }
}










#endif