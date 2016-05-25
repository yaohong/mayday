#ifndef _Endian_h__
#define _Endian_h__

namespace mayday
{
    namespace net
    {
        namespace sockets
        {
            inline uint64 hostToNetwork64( uint64 host64 )
            {
#ifndef WIN32
                return htobe64( host64 );
#else 
                return htonll( host64 );
#endif
            }

            inline uint32 hostToNetwork32( uint32 host32 )
            {
#ifndef WIN32
                return htobe32( host32 );
#else 
                return htonl( host32 );
#endif
            }

            inline uint16 hostToNetwork16( uint16 host16 )
            {
#ifndef WIN32
                return htobe16( host16 );
#else 
                return htons( host16 );
#endif
            }

            inline uint64 networkToHost64( uint64 net64 )
            {
#ifndef WIN32
                return be64toh( net64 );
#else 
                return ntohll( net64 );
#endif
            }

            inline uint32 networkToHost32( uint32 net32 )
            {
#ifndef WIN32
                return be32toh( net32 );
#else 
                return ntohl( net32 );
#endif
            }

            inline uint16 networkToHost16( uint16 net16 )
            {
#ifndef WIN32
                return be16toh( net16 );
#else 
                return ntohs( net16 );
#endif
            }
        }
    }
}











#endif