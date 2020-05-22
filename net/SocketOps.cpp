
#include "../base/Define.h"
#include "SocketOps.h"
#include "Endian.h"
using namespace mayday;
using namespace mayday::net;

void sockets::setNonBlockAndCloseOnExec( int sockfd )
{
#ifndef WIN32
    // non-block
    int flags = ::fcntl( sockfd, F_GETFL, 0 );
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    // FIXME check

    // close-on-exec
    flags = ::fcntl( sockfd, F_GETFD, 0 );
    flags |= FD_CLOEXEC;
    ret = ::fcntl( sockfd, F_SETFD, flags );
    // FIXME check

    (void)ret;
#else
    int ul = 1;           //1·Ç×èÈû:0×èÈû
    if (SOCKET_ERROR == ioctlsocket( sockfd, FIONBIO, (unsigned long *)&ul ))
    {
        MDAssert( false );
    }
#endif
}


int sockets::createNonblockingOrDie()
{
#ifndef WIN32 
    int sockfd = ::socket( PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0 );
    MDAssert( !(sockfd < 0) );
    return sockfd;
#else
    int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
    MDAssert( !(sockfd < 0) );
    sockets::setNonBlockAndCloseOnExec( sockfd );
    return sockfd;
#endif
}

const struct sockaddr* sockets::sockaddr_cast( const struct sockaddr_in* addr )
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

struct sockaddr* sockets::sockaddr_cast( struct sockaddr_in* addr )
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));
}

const struct sockaddr_in* sockets::sockaddr_in_cast( const struct sockaddr* addr )
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

void sockets::toIpPort( char* buf, size_t size, const struct sockaddr* addr )
{
    toIp( buf, size, addr );
    size_t end = ::strlen( buf );
    const struct sockaddr_in* addr4 = sockaddr_in_cast( addr );
    uint16 port = sockets::networkToHost16( addr4->sin_port );
    assert( size > end );
#ifndef WIN32
    snprintf( buf + end, size - end, ":%u", port );
#else 
    sprintf_s( buf + end, size - end, ":%u", port );
#endif
}

void sockets::toIp( char* buf, size_t size, const struct sockaddr* addr )
{
    assert( size >= INET_ADDRSTRLEN );
    const struct sockaddr_in* addr4 = sockets::sockaddr_in_cast( addr );
    ::inet_ntop( AF_INET, (void *)&(addr4->sin_addr), buf, static_cast<socklen_t>(size) );
}

int  sockets::connect( int sockfd, const struct sockaddr* addr )
{
    return ::connect( sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)) );
}
void sockets::bind( int sockfd, const struct sockaddr* addr )
{
    int ret = ::bind( sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in)) );
    MDAssert( !(ret < 0) );
}
void sockets::listen( int sockfd )
{
    int ret = ::listen( sockfd, SOMAXCONN );
    MDAssert( !(ret < 0) );
}

int sockets::accept( int sockfd, struct sockaddr_in* addr )
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);
    int connfd = ::accept( sockfd, sockaddr_cast( addr ), &addrlen );
    return connfd;
}

//int sockets::read( int sockfd, void *buf, size_t count )
//{
//#ifdef WIN32
//    return ::read( sockfd, buf, count );
//}
//
//int sockets::write( int sockfd, const void *buf, size_t count )
//{
//    return ::write( sockfd, buf, count );
//}

void sockets::close( int sockfd )
{
#ifndef WIN32
    ::close( sockfd );
#else 
    closesocket( sockfd );
#endif
}

struct sockaddr_in sockets::getLocalAddr( int sockfd )
{
    struct sockaddr_in localaddr;
    memset( &localaddr, 0, sizeof localaddr );
    socklen_t addrlen = static_cast<socklen_t>(sizeof localaddr);
    if (::getsockname( sockfd, sockaddr_cast( &localaddr ), &addrlen ) < 0)
    {
        MDAssert( false );
    }
    return localaddr;
}

struct sockaddr_in sockets::getPeerAddr( int sockfd )
{
    struct sockaddr_in peeraddr;
    memset( &peeraddr, 0, sizeof peeraddr );
    socklen_t addrlen = static_cast<socklen_t>(sizeof peeraddr);
    if (::getpeername( sockfd, sockaddr_cast( &peeraddr ), &addrlen ) < 0)
    {
        MDAssert( false );
    }
    return peeraddr;
}

int sockets::getSocketError( int sockfd )
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt( sockfd, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen ) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}


void sockets::setSendBuffSize( int sockfd, int size )
{
    int ret = ::setsockopt( sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size) );
    if (ret < 0)
    {
        MDAssert( false );
    }
}

void sockets::setRecvBuffSize( int sockfd, int size )
{
    int ret = ::setsockopt( sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&size, sizeof(size) );
    if (ret < 0)
    {
        MDAssert( false );
    }
}