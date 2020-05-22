#include "TimerManager.h"
#include "Timestamp.h"

namespace mayday
{

    TimerManager::TimerManager()
        : wheels_( WHEEL_NUM )
    {
        checkTime_ = Timestamp::now().milliSecondsSinceEpoch();
//        checkTime_ = 1;
        nowTime_ = 0;
        std::unique_ptr<Wheel> w0( new Wheel( WHEEL_SIZE1 ) );
        wheels_[0] = std::move(w0);
        for (int i = 1; i < WHEEL_NUM; i++)
        {
            std::unique_ptr<Wheel> w( new Wheel( WHEEL_SIZE2 ) );
            wheels_[i] = std::move( w );
        }

        idAlloc_ = 0;
    }

    TimerManager::~TimerManager()
    {

    }

    uint64 TimerManager::runAfter( uint32 milseconds, const Functor  &cb )
    {
        int64 deadTime = Timestamp::now().milliSecondsSinceEpoch() + milseconds;
        Timer *timer = new Timer( idAlloc_++, cb, deadTime, DisposableTimer, milseconds );
        timerMap_[timer->timerId_] = timer;
        addTimerNode( milseconds, timer );
        return timer->timerId_;
    }
    
    uint64 TimerManager::runEvery( uint32 milseconds, const Functor  &cb )
    {
        int64 deadTime = Timestamp::now().milliSecondsSinceEpoch() + milseconds;
        Timer *timer = new Timer( idAlloc_++, cb, deadTime, EveryTimer, milseconds );
        timerMap_[timer->timerId_] = timer;
        addTimerNode( milseconds, timer );
        return timer->timerId_;
    }

    void TimerManager::removeTimer( uint64 timerId )
    {
        std::map<uint64, Timer *>::iterator p_it = timerMap_.find( timerId );
        if (p_it != timerMap_.end())
        {
            Timer *timer = p_it->second;
            timerMap_.erase( p_it );


            TimerLink *nodeLink = &(timer->link_);
            assert( nodeLink->prev_ );
            assert( nodeLink->next_ );

            nodeLink->prev_->next_ = nodeLink->next_;
            nodeLink->next_->prev_ = nodeLink->prev_;

            delete timer;
        }
    }

    void TimerManager::detectTimer()
    {
        Timestamp now = Timestamp::now();
        detectTimer( now );
    }

    void TimerManager::detectTimer( Timestamp &now )
    {
        nowTime_ = now.milliSecondsSinceEpoch();
        int64 loopNum = nowTime_ > checkTime_ ? (nowTime_ - checkTime_) / GRANULARITY : 0;

        std::unique_ptr<Wheel>& wheel = wheels_[0];
        for (int64 i = 0; i < loopNum; i++)
        {
            //将当前刻度的timer全部放入就绪列表
            TimerLink *spoke = &(wheel->spokes_[wheel->spokeIndex_]);
            TimerLink *link = spoke->next_;
            while (link != spoke)
            {
                Timer *node = (Timer *)link;
                link = link->next_;
                addTimerToReady( node );
            }
            spoke->next_ = spoke->prev_ = spoke;

            if (++(wheel->spokeIndex_) >= wheel->size_)
            {
                //已经跑完一圈了，带动下一个轮子
                wheel->spokeIndex_ = 0;
                int32 cascadeCount = cascade( 1 );
                //MDLog( "cascadeCount=%d", cascadeCount );
            }

            checkTime_ += GRANULARITY;
        }

        doReadyTimer();
    }

    void TimerManager::addTimerNode( uint32 milseconds, Timer *timer )
    {
        TimerLink *spoke = NULL;
        uint32 interval = milseconds / GRANULARITY;
        uint32_t threshold1 = WHEEL_SIZE1;
        uint32_t threshold2 = 1 << (WHEEL_BITS1 + WHEEL_BITS2);
        uint32_t threshold3 = 1 << (WHEEL_BITS1 + 2 * WHEEL_BITS2);
        uint32_t threshold4 = 1 << (WHEEL_BITS1 + 3 * WHEEL_BITS2);

        if (interval < threshold1)
        {
            uint32_t index = (interval + wheels_[0]->spokeIndex_) & WHEEL_MASK1;
            spoke = &(wheels_[0]->spokes_[index]);
        }
        else if (interval < threshold2)
        {
            uint32_t index = ((interval - threshold1 + wheels_[1]->spokeIndex_ * threshold1) >> WHEEL_BITS1) & WHEEL_MASK2;
            spoke = &(wheels_[1]->spokes_[index]);
        }
        else if (interval < threshold3)
        {
            uint32_t index = ((interval - threshold2 + wheels_[2]->spokeIndex_ * threshold2) >> (WHEEL_BITS1 + WHEEL_BITS2)) & WHEEL_MASK2;
            spoke = &(wheels_[2]->spokes_[index]);

        }
        else if (interval < threshold4)
        {
            uint32_t index = ((interval - threshold3 + wheels_[3]->spokeIndex_ * threshold3) >> (WHEEL_BITS1 + 2 * WHEEL_BITS2)) & WHEEL_MASK2;
            spoke = &(wheels_[3]->spokes_[index]);
        }
        else
        {
            uint32_t index = ((interval - threshold4 + wheels_[4]->spokeIndex_ * threshold4) >> (WHEEL_BITS1 + 3 * WHEEL_BITS2)) & WHEEL_MASK2;
            spoke = &(wheels_[4]->spokes_[index]);
        }

        TimerLink *nodeLink = &(timer->link_);
        nodeLink->prev_ = spoke->prev_;
        nodeLink->next_ = spoke;

        spoke->prev_->next_ = nodeLink;
        spoke->prev_ = nodeLink;
    }

    int32 TimerManager::cascade( uint32 wheelIndex )
    {
        assert( wheelIndex >= 1);
        if (wheelIndex >= WHEEL_NUM)
        {
            return 0;
        }
        int32 casnum = 0;
        std::unique_ptr<Wheel>& wheel = wheels_[wheelIndex];

        TimerLink *spoke = &(wheel->spokes_[wheel->spokeIndex_++]);
        TimerLink *link = spoke->next_;
        while (link != spoke)
        {
            Timer *node = (Timer *)link;
            link = link->next_;
            if (node->deadTime_ <= nowTime_)
            {
                addTimerToReady( node );
            }
            else
            {
                int64 spaceTime = node->deadTime_ - nowTime_;
                addTimerNode( static_cast<uint32>(spaceTime), node );
                casnum++;
            }
        }
        spoke->next_ = spoke->prev_ = spoke;

        if (wheel->spokeIndex_ >= wheel->size_)
        {
            wheel->spokeIndex_ = 0;
            casnum += cascade( ++wheelIndex );
        }

        return casnum;
    }

    void TimerManager::addTimerToReady( Timer *timer )
    {
        assert(timer);
        readyTimers_.push_back(timer);
    }
    void TimerManager::doReadyTimer()
    {
        std::vector<Timer *>::iterator p_it = readyTimers_.begin();
        for (; p_it != readyTimers_.end(); ++p_it)
        {
            Timer *timer = *p_it;
            //执行函数
            timer->cb_();
            //从hash表里移除
            if (timer->timerType_ == DisposableTimer)
            {
                //只执行一次
                timerMap_.erase( timer ->timerId_);
                delete timer;
            }
            else if (timer->timerType_ == EveryTimer)
            {
                //隔一段时间执行,重新添加
                timer->deadTime_ = nowTime_ + timer->interval_;
                addTimerNode( static_cast<uint32>(timer->interval_), timer );
            }
            else
            {
                assert(false);
            }
        }

        readyTimers_.clear();
    }
}