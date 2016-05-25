

#include "InetAddress.h"
#include "SocketOps.h"

namespace mayday
{
    namespace net
    {
        InetAddress::InetAddress( uint16 port )
        {
            memset( &addr_, 0, sizeof addr_ );
            addr_.sin_family = AF_INET;
            addr_.sin_addr.s_addr = htonl( INADDR_ANY );
            addr_.sin_port = htons( port );
        }


        InetAddress::InetAddress( const std::string& ip, uint16 port )
        {
            memset( &addr_, 0, sizeof addr_ );
            addr_.sin_family = AF_INET;
            addr_.sin_addr.s_addr = inet_addr(ip.c_str());
            addr_.sin_port = htons( port );
        }

        std::string InetAddress::toIpPort() const
        {
            char buf[64] = "";
            sockets::toIpPort( buf, sizeof buf, getSockAddr() );
            return buf;
        }

        std::string InetAddress::toIp( ) const
        {
            char buf[64] = "";
            sockets::toIp( buf, sizeof buf, getSockAddr() );
            return buf;
        }

    }
}
