#ifndef _Socket_h__
#define _Socket_h__



namespace mayday
{
    namespace net
    {
        class InetAddress;
        class Socket
        {
        public:
            explicit Socket( int sockfd )
                : sockfd_( sockfd )
            {
            }
            ~Socket();

            int fd() const { return sockfd_; }

            void bindAddress( const InetAddress  &localAddr);
            void listen();
            int accept( InetAddress* peeraddr );

            void setTcpNoDelay( bool on );
            void setReuseAddr( bool on );
            void setKeepAlive( bool on );
            void setSendBuffSize( int size );
            void setRecvBuffSize( int size );
        private:
            const int sockfd_;
        };
    }
}












#endif