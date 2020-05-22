#include "Acceptor.h"
#include "SocketOps.h"
#include "InetAddress.h"
#include "NetworkLoop.h"
namespace mayday
{
    namespace net
    {
        Acceptor::Acceptor( NetworkLoop  *loop, const InetAddress& listenAddr )
            : loop_( loop )
            , acceptSocket_( sockets::createNonblockingOrDie() )
            , acceptChannel_( loop, acceptSocket_.fd() )
        {
            acceptSocket_.setReuseAddr( true );
            acceptSocket_.bindAddress( listenAddr );
            acceptChannel_.setReadCallback( std::bind( &Acceptor::handleRead, this ) );
        }

        Acceptor::~Acceptor()
        {
            acceptChannel_.disableAll();
            acceptChannel_.remove();
        }

        void Acceptor::listen()
        {
            acceptSocket_.listen();
            acceptChannel_.enableReading();
        }

        void Acceptor::handleRead()
        {
            loop_->assertInLoopThread();
            InetAddress peerAddr;
            int connfd = acceptSocket_.accept( &peerAddr );
            if (connfd >= 0)
            {
                sockets::setNonBlockAndCloseOnExec( connfd );
                MDLog( "peerIp:%s, peerPort:%s", peerAddr.toIp( ).c_str(), peerAddr.toIpPort().c_str());
                if (newConnectionCallback_)
                {
                    newConnectionCallback_( connfd, peerAddr );
                }
                else
                {
                    sockets::close( connfd );
                }
            }
            else
            {

                if (errno == EAGAIN)
                {
                    return;
                }

                MDError( "accept error [%d]", errno );
            }
        }
    }
}
