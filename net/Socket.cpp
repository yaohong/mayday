
#include "../base/Define.h"
#include "Socket.h"
#include "SocketOps.h"
#include "InetAddress.h"
namespace mayday
{
    namespace net
    {
        Socket::~Socket()
        {
            sockets::close( sockfd_ );
        }

        void Socket::bindAddress( const InetAddress  &localAddr )
        {
            sockets::bind( sockfd_, localAddr.getSockAddr() );
        }

        void Socket::listen()
        {
            sockets::listen( sockfd_ );
        }

        int Socket::accept( InetAddress* peeraddr )
        {
            struct sockaddr_in addr;
            memset( &addr, 0, sizeof addr);
            int connfd = sockets::accept( sockfd_, &addr );
            if (connfd >= 0)
            {
                peeraddr->setSockAddrInet( addr );
            }
            return connfd;
        }

        void Socket::setTcpNoDelay( bool on )
        {
            int optval = on ? 1 : 0;
#ifdef WIN32
            if (-1 == setsockopt( sockfd_, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, sizeof(optval) ))
            {
                MDAssert( false );
            }
#else

            if (-1 == setsockopt( sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval) ))
            {
                MDAssert( false );
            }
#endif
        }

        void Socket::setReuseAddr( bool on )
        {
            int optval = on ? 1 : 0;
#ifdef WIN32
            if (-1 == setsockopt( sockfd_, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval) ))
            {
                MDAssert( false );
            }
#else

            if (-1 == setsockopt( sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval) ))
            {
                MDAssert( false );
            }
#endif
        }

        void Socket::setKeepAlive( bool on )
        {
            int optval = on ? 1 : 0;
#ifdef WIN32
            if (-1 == setsockopt( sockfd_, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval) ))
            {
                MDAssert( false );
            }
#else

            if (-1 == setsockopt( sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval) ))
            {
                MDAssert( false );
            }
#endif
        }

        void Socket::setSendBuffSize( int size )
        {
            sockets::setSendBuffSize( sockfd_, size );
        }
        void Socket::setRecvBuffSize( int size )
        {
            sockets::setRecvBuffSize( sockfd_, size );
        }
    }
}