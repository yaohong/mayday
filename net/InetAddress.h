
#ifndef _InetAddress_h__
#define _InetAddress_h__

#include "../base/Define.h"
namespace mayday
{
    namespace net
    {
        namespace sockets
        {
            const struct sockaddr* sockaddr_cast( const struct sockaddr_in* addr );
        }

        class InetAddress
        {
        public:
            explicit InetAddress( uint16 port = 0 );
            InetAddress(const std::string& ip, uint16 port );

            explicit InetAddress( const struct sockaddr_in& addr )
                : addr_( addr )
            {
            }

            std::string toIp() const;
            std::string toIpPort() const;
            uint16 toPort() const;

            // default copy/assignment are Okay

            const struct sockaddr* getSockAddr( ) const { return sockets::sockaddr_cast( &addr_ ); }
            void setSockAddrInet( const struct sockaddr_in& addr ) { addr_ = addr; }
        private:
            struct sockaddr_in addr_;
        };

    }
}

#endif  // MUDUO_NET_INETADDRESS_H
