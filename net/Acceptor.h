#ifndef _Acceptor_h__
#define _Acceptor_h__

#include "Channel.h"
#include "Socket.h"
namespace mayday
{
    namespace net
    {
        class NetworkLoop;
        class Acceptor
        {
        public:
            Acceptor( NetworkLoop  *loop, const InetAddress& listenAddr );
            ~Acceptor();

            typedef std::function<void( int sockfd, const InetAddress& )> NewConnectionCallback;

            void setNewConnectionCallback( const NewConnectionCallback& cb )
            {
                newConnectionCallback_ = cb;
            }

            void listen( );
        private:
            void handleRead();

            NetworkLoop *loop_;
            Socket acceptSocket_;
            Channel acceptChannel_;
            NewConnectionCallback newConnectionCallback_;
        };
    }
}











#endif