#ifndef _TimerManager_h__
#define _TimerManager_h__

#include "Define.h"
#include "Timestamp.h"
namespace mayday
{
    #define GRANULARITY 10 //10ms
    #define WHEEL_BITS1 8
    #define WHEEL_BITS2 6
    #define WHEEL_SIZE1 (1 << WHEEL_BITS1) //256
    #define WHEEL_SIZE2 (1 << WHEEL_BITS2) //64
    #define WHEEL_MASK1 (WHEEL_SIZE1 - 1)
    #define WHEEL_MASK2 (WHEEL_SIZE2 - 1)
    #define WHEEL_NUM 5
    typedef std::function<void()> Functor;

    struct TimerLink
    {
        TimerLink *prev_;
        TimerLink *next_;
        TimerLink()
        {
            prev_ = next_ = this;
        }
    };

    enum TimerType
    {
        DisposableTimer = 1,            //执行一次
        EveryTimer = 2                  //循环执行
    };

    struct Timer
    {
        TimerLink link_;
        uint64 timerId_;            //定时器id
        int64 deadTime_;
        Functor cb_;
        TimerType timerType_;           //定时器类型
        int64 interval_;                //间隔时间
        Timer( uint64 timerId, const Functor &cb, int64 deadTime, TimerType  tt, int64 interval )
            : timerId_( timerId ) 
            , deadTime_( deadTime )
            , cb_( cb )
            , timerType_( tt )
            , interval_( interval )
        {
            
        }
    };

    class Wheel
    {
    public:
        Wheel( uint32 size ) 
            : size_( size )
            , spokeIndex_( 0 )
            , spokes_( size )
        {
            MDLog("wheel cap:%d, size:%d", spokes_.capacity(), spokes_.size());
        };
        ~Wheel() 
        {
            for (uint32 i = 0; i < size_; i++)
            {    
                TimerLink *link = spokes_[i].next_;
                while (link != &(spokes_[i]))
                {
                    Timer *timer = (Timer *)link;
                    link = link->next_;
                    delete timer;
                }
            }
        }

        std::vector<TimerLink > spokes_;
        uint32 size_;
        uint32 spokeIndex_;

    };
    class TimerManager
    {
    public:
        TimerManager();
        ~TimerManager();
    public:
        void detectTimer(Timestamp &now);
        void detectTimer();
        void removeTimer( uint64 timerId );
        //延时执行
        uint64 runAfter( uint32 milseconds, const Functor  &cb );
        //循环执行
        uint64 runEvery( uint32 milseconds, const Functor  &cb );
    private:
        int32 cascade(uint32 wheelIndex);
        void addTimerToReady( Timer *timer);
        void doReadyTimer();
        void addTimerNode( uint32 milseconds, Timer *timer );
    private:
        int64_t checkTime_;
        int64_t nowTime_;
        std::vector<std::unique_ptr<Wheel> > wheels_;
        std::vector<Timer *> readyTimers_;

    private:
        uint64 idAlloc_;
        std::map<uint64, Timer * > timerMap_;
    };
}











#endif