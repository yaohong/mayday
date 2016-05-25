

#ifndef _Channel_h__
#define _Channel_h__

#include "../base/Define.h"


namespace mayday
{
    namespace net
    {
        class NetworkLoop;
        class Channel
        {
            friend class TcpConnection;

        public:
            typedef std::function<void()> EventCallback;
            typedef std::function<void()> ReadEventCallback;

            Channel( NetworkLoop* loop, int fd );
            ~Channel();

            void handleEvent();
            void setReadCallback( const ReadEventCallback& cb )
            {
                readCallback_ = cb;
            }
            void setWriteCallback( const EventCallback& cb )
            {
                writeCallback_ = cb;
            }
            void setCloseCallback( const EventCallback& cb )
            {
                closeCallback_ = cb;
            }
            void setErrorCallback( const EventCallback& cb )
            {
                errorCallback_ = cb;
            }


            int fd() const { return fd_; }
            int events() const { return events_; }
            void set_revents( int revt ) { revents_ = revt; }

            bool isNoneEvent() const { return events_ == kNoneEvent; }

            void enableReading() {  events_ |= kReadEvent; update(); }
            void disableReading() { events_ &= ~kReadEvent; update(); }
            void enableWriting() { events_ |= kWriteEvent; update(); }
            void disableWriting() { events_ &= ~kWriteEvent; update(); }
            void disableAll() { events_ = kNoneEvent; update(); }
            bool isWriting() const { return events_ & kWriteEvent; }
            bool isReading() const { return events_ & kReadEvent; }


            int index() { return index_; }
            void set_index( int idx ) { index_ = idx; }


            NetworkLoop *ownerLoop() { return loop_; }
            void remove();

            std::string reventsToString() const;
            std::string eventsToString() const;
        private:
            void update();
            static std::string eventsToString( int fd, int ev );
            static const int kNoneEvent;
            static const int kReadEvent;
            static const int kWriteEvent;

            const int fd_;
            int        events_;
            int        revents_;
            int        index_;

            NetworkLoop* loop_;

            ReadEventCallback readCallback_;
            EventCallback writeCallback_;
            EventCallback closeCallback_;
            EventCallback errorCallback_;

            bool eventHandling_;
            bool addedToLoop_;
        };
    }
}

#endif  
