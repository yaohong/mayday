#ifndef _Atomic_h__
#define _Atomic_h__
#include <atomic>
namespace mayday
{
    namespace detail
    {
        template<typename T>
        class AtomicIntegerT
        {
        public:
            AtomicIntegerT()
            {
                
            }
        public:
            T get()
            {
                return value_.load();
            }

            //�ȷ���֮ǰ��ֵ��+
            T getAndAdd( T x )
            {
                return value_.fetch_add(x);
            }

            //�����ڷ�����ֵ
            T addAndGet( T x )
            {
                return getAndAdd( x ) + x;
            }

            
            T incrementAndGet()
            {
                return addAndGet( 1 );
            }

            T decrementAndGet()
            {
                return addAndGet( -1 );
            }

            void add( T x )
            {
                getAndAdd( x );
            }

            void increment()
            {
                incrementAndGet();
            }

            void decrement()
            {
                decrementAndGet();
            }

            //��ȡ֮ǰ��ֵȻ���ڸ�����ֵ
            T getAndSet( T newValue )
            {
                return value_.exchange( newValue );
            }

        private:
            std::atomic<T> value_;
        };
    }

    typedef detail::AtomicIntegerT<int32_t> AtomicInt32;
    typedef detail::AtomicIntegerT<int64_t> AtomicInt64;
}











#endif