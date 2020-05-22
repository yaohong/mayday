#include "Timestamp.h"
#include <time.h>
#include <inttypes.h>

#ifdef WIN32
int gettimeofday( struct timeval *tp, void *tzp )
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime( &wtm );
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime( &tm );
    tp->tv_sec = clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#endif


namespace mayday
{
    std::string Timestamp::toString() const
    {
        char buf[32] = { 0 };
        int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondsPerSecond;
        int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondsPerSecond;


        //STRING_FMT( buf, sizeof(buf)-1, "%"PRId64".%06"PRId64 "", seconds, microseconds );
        STRING_FMT(buf, sizeof(buf)-1, "%lld.%06lld", (int64)seconds, (int64)microseconds);
        return buf;
    }

    //将unix时间戳转换成字符串格式
    std::string Timestamp::toFormattedString( bool showMicroseconds ) const
    {
        char buf[64] = { 0 };
        time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
#ifndef WIN32 
        
        struct tm tm_time;
        gmtime_r( &seconds, &tm_time );
#else 
        struct tm *tmp_tm_time;
        tmp_tm_time = gmtime( &seconds );
        struct tm tm_time = *tmp_tm_time;
#endif

        if (showMicroseconds)
        {
            int microseconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondsPerSecond);
            STRING_FMT( buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%06d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec,
                microseconds );
        }
        else
        {
            STRING_FMT( buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
                tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec );
        }
        return buf;
    }
    static int64 testTime = 1;
    Timestamp Timestamp::now()
    {
        struct timeval tv;
        gettimeofday( &tv, NULL );
        int64_t seconds = tv.tv_sec;
        return Timestamp( seconds * kMicroSecondsPerSecond + tv.tv_usec );
        //return Timestamp( (testTime++) * kMicroSecondsPerSecond );
    }
}