
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

        //����ʱ��
        void swap( Timestamp& that )
        {
            std::swap( microSecondsSinceEpoch_, that.microSecondsSinceEpoch_ );
        }

        //��ʽ��ʱ��
        std::string toString() const;
        std::string toFormattedString( bool showMicroseconds = true ) const;

        bool valid() const { return microSecondsSinceEpoch_ > 0; }

        //��ǰ΢��
        int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
        //��ǰ����
        int64_t milliSecondsSinceEpoch() const { return static_cast<int64_t>(microSecondsSinceEpoch_ / (kMicroSecondsPerSecond / 1000));  }
        //��ǰ��
        int64_t secondsSinceEpoch() const {return static_cast<int64_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);}
        static Timestamp now();

        //����һ����ʱ�����
        static Timestamp invalid()
        {
            return Timestamp();
        }

        //��unixʱ���ת����΢����
        static Timestamp fromUnixTime( time_t t )
        {
            return fromUnixTime( t, 0 );
        }

        //��unixʱ���ת����΢����
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

    //��ȡ2��ʱ����������
    inline double timeDifference( Timestamp high, Timestamp low )
    {
        int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
        return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
    }

    //��ȡ2��ʱ�����ĺ�����
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