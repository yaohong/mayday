
#ifndef _Timestamp_h__
#define _Timestamp_h__
#include <stdint.h>
#include "Define.h"
namespace mayday
{
    class Timestamp
    {
    public:
        Timestamp()
            : microSecondsSinceEpoch_( 0 )
        {
        }

        explicit Timestamp( int64_t microSecondsSinceEpochArg )
            : microSecondsSinceEpoch_( microSecondsSinceEpochArg )
        {
        }

        //交换时间
        void swap( Timestamp& that )
        {
            std::swap( microSecondsSinceEpoch_, that.microSecondsSinceEpoch_ );
        }

        //格式化时间
        std::string toString() const;
        std::string toFormattedString( bool showMicroseconds = true ) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        //当前微秒
        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        //当前毫秒
        int64_t milliSecondsSinceEpoch() const { return static_cast<int64_t>(microSecondsSinceEpoch_ / (kMicroSecondsPerSecond / 1000));  }
        //当前秒
        int64_t secondsSinceEpoch() const {return static_cast<int64_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);}
        static Timestamp now();

        //创建一个空时间对象
        static Timestamp invalid()
        {
            return Timestamp();
        }

        //将unix时间戳转换成微秒数
        static Timestamp fromUnixTime( time_t t )
        {
            return fromUnixTime( t, 0 );
        }

        //将unix时间戳转换成微秒数
        static Timestamp fromUnixTime( time_t t, int microseconds )
        {
            return Timestamp( static_cast<int64_t>(t)* kMicroSecondsPerSecond + microseconds );
        }

        static const int kMicroSecondsPerSecond = 1000 * 1000;
    private:
    private:
        int64_t microSecondsSinceEpoch_;
    };

    inline bool operator<(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

    //获取2个时间相差的秒数
    inline double timeDifference( Timestamp high, Timestamp low )
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    //获取2个时间相差的毫秒数
    inline double timeMSDifference( Timestamp high, Timestamp low )
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / (Timestamp::kMicroSecondsPerSecond / 1000);
    }

    inline Timestamp addTime( Timestamp timestamp, double seconds )
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp( timestamp.microSecondsSinceEpoch() + delta );
    }
}









#endif