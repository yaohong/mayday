#ifndef _SocketOps_h__
#define _SocketOps_h__

namespace mayday
{
    namespace net
    {
        namespace sockets
        {
            void setNonBlockAndCloseOnExec( int sockfd );
            int createNonblockingOrDie();

            const struct sockaddr* sockaddr_cast( const struct sockaddr_in* addr );
            struct sockaddr* sockaddr_cast( struct sockaddr_in* addr );
            const struct sockaddr_in* sockaddr_in_cast( const struct sockaddr* addr );
            void toIpPort( char* buf, size_t size, const struct sockaddr* addr );
            void toIp( char* buf, size_t size, const struct sockaddr* addr );
            void setSendBuffSize( int sockfd, int size );
            void setRecvBuffSize( int sockfd, int size );
            int  connect( int sockfd, const struct sockaddr* addr );
            void bind( int sockfd, const struct sockaddr* addr );
            void listen( int sockfd );
            int  accept( int sockfd, struct sockaddr_in* addr );
            void close( int sockfd );

            struct sockaddr_in getLocalAddr( int sockfd );
            struct sockaddr_in getPeerAddr( int sockfd );
            int getSocketError( int sockfd );
        }
    }
}







#endif