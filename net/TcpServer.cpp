
#include "TcpServer.h"
#include "NetworkLoop.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "SocketOps.h"
#include "TcpConnection.h"

namespace mayday
{
    namespace net
    {
        TcpServer::TcpServer( NetworkLoop *loop, const std::string &name, const InetAddress &listenAddr, int32 recvSize, int32 sendSize )
            : loop_( loop )
            , name_( name )
            , ipPort_( listenAddr.toIpPort() )
            , acceptor_( loop, listenAddr )
            , nextConnId_( 1 )
            , started_( false )
            , recvBufferSize_( recvSize )
            , sendBuffSize_( sendSize )
        {
            acceptor_.setNewConnectionCallback( std::bind( &TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2 ) );
        }

        TcpServer::~TcpServer()
        {
            loop_->assertInLoopThread();
            for (ConnectionMap::iterator it( connections_.begin() ); it != connections_.end(); ++it)
            {
                TcpConnectionPtr conn = it->second;
                it->second.reset();
                conn->getLoop()->runInLoop( std::bind( &TcpConnection::connectDestroyed, conn ) );
                conn.reset();
            }
        }

        void TcpServer::start()
        {
            if (!started_)
            {
                started_ = true;
                loop_->runInLoop( std::bind( &Acceptor::listen, &acceptor_ ) );
            }
        }

        void TcpServer::newConnection( int sockfd, const InetAddress& peerAddr )
        {
            loop_->assertInLoopThread();
            char buf[64];

            STRING_FMT( buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_++ );

            std::string connName = name_ + buf;
            MDLog( "TcpServer[%s], new Connection[%s] from[%s]", name_.c_str(), connName.c_str(), peerAddr.toIpPort().c_str() );
            InetAddress localAddr( sockets::getLocalAddr( sockfd ) );
            //���̻߳���ֱ��ʹ��tcpServer���ڵ�loop
            TcpConnectionPtr conn( new TcpConnection( loop_, connName, sockfd, localAddr, peerAddr, recvBufferSize_, sendBuffSize_ ) );
            connections_[connName] = conn;
            conn->setConnectionCallback( connectionCallback_ );
            conn->setMessageCallback( messageCallback_ );
            conn->setWriteCompleteCallback( writeCompleteCallback_ );

            //���ӹر�ʱ��Ҫ֪ͨ������TcpServer
            conn->setCloseCallback( std::bind( &TcpServer::removeConnection, this, std::placeholders::_1 ) );

            loop_->runInLoop( std::bind( &TcpConnection::connectEstablished, conn ) );
        }

        //��TcpConnectionPtr���̵߳����������ܺ�TcpServer���ڵ��̲߳�һ����
        void TcpServer::removeConnection( const TcpConnectionPtr& conn )
        {
            loop_->runInLoop( std::bind( &TcpServer::removeConnectionInLoop, this, conn ) );
        }

        void TcpServer::removeConnectionInLoop( const TcpConnectionPtr& conn )
        {
            loop_->assertInLoopThread();
            MDLog( "TcpServer[%s] removeConnection[%s].", name_.c_str(), conn->name().c_str() );
            size_t n = connections_.erase( conn->name() );
            assert( n == 1 );
            NetworkLoop* ioLoop = conn->getLoop();
            ioLoop->queueInLoop( std::bind( &TcpConnection::connectDestroyed, conn ) );
        }

    }
}