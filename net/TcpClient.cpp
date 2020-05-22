
#include "TcpClient.h"
#include "Connector.h"
#include "TcpConnection.h"
#include "SocketOps.h"
#include "NetworkLoop.h"

namespace mayday
{
	namespace net
	{
        TcpClient::TcpClient( NetworkLoop *loop )
            : loop_(loop)
            , nextConnectorId_(1)
            , nextConnectionId_(1)
		{

		}

		TcpClient::~TcpClient()
		{
            for (ConnectionMap::iterator it( connections_.begin() );
                it != connections_.end(); ++it)
            {
                TcpConnectionPtr conn = it->second;
                it->second.reset();
                conn->connectDestroyed();
                conn.reset();
            }

            for (ConnectorMap::iterator it( connectors_.begin() );
                it != connectors_.end(); ++it)
            {
                std::unique_ptr<ConnectorContext> &conn = it->second;
                conn->connectorPtr_->stop();
                it->second.reset();
            }
		}

		void TcpClient::connect(
            const std::string &name,
			const InetAddress &serverAddr,									//Ŀ��������ĵ�ַ
			const ConnectFailedCallback &connectFailedCallback,				//����ʧ�ܵĻص�����
			const ConnectSuccessCallback &connectSuccessCallback,			//���ӳɹ��Ļص�����	

			const ConnectionCallback &connectionCallback,					//����״̬�仯�Ļص�(ֻ�����ӶϿ�ʱ�����)
			const MessageCallback &messageCallback,							//����Ϣ�Ļص�����
			const WriteCompleteCallback &writeCompleteCallback,				//����д��ϡ�Ļص�����
			int32  timeoutSecond,                                               //���ӵĳ�ʱʱ��
            int32 recvBufferSize,                                            //ϵͳ���ջ����С
            int32 sendBufferSize,                                           //ϵͳ���ͻ����С
			int32 cacheRecvBuferSize,										//Buffer�Ľ��ջ����С
			int32 cacheSendBufferSize										//Buffer�ķ��ͻ����С
			)
		{

            char buf[64];
#ifndef WIN32
            snprintf( buf, sizeof buf, "connector#%d", nextConnectorId_++ );
#else 
            sprintf_s( buf, sizeof buf, "connector#%d", nextConnectorId_++ );
#endif
            std::string connectorName =  buf;
//            MDLog( "TcpClient connector[%s] to[%s]", connectorName.c_str(), serverAddr.toIpPort().c_str() );
            ConnectorPtr connector( new Connector( loop_, connectorName, serverAddr, timeoutSecond ) );
			std::unique_ptr<ConnectorContext> connContext(new ConnectorContext(name, connector, connectFailedCallback, connectSuccessCallback, connectionCallback, messageCallback, writeCompleteCallback, recvBufferSize, sendBufferSize, cacheRecvBuferSize, cacheSendBufferSize));
            connectors_[connectorName] = std::move( connContext );
            connector->setConnectCompleteCallback( std::bind( &TcpClient::connectComplete, this, std::placeholders::_1, std::placeholders::_2 ) );

            connector->start();
		}

        void TcpClient::delayConnect(
            const std::string &name,
            const InetAddress &serverAddr,									//Ŀ��������ĵ�ַ
            const ConnectFailedCallback &connectFailedCallback,				//����ʧ�ܵĻص�����
            const ConnectSuccessCallback &connectSuccessCallback,			//���ӳɹ��Ļص�����	

            const ConnectionCallback &connectionCallback,					//����״̬�仯�Ļص�(ֻ�����ӶϿ�ʱ�����)
            const MessageCallback &messageCallback,							//����Ϣ�Ļص�����
            const WriteCompleteCallback &writeCompleteCallback,				//����д��ϡ�Ļص�����
            int timeoutSecond,
            int delaySecond,
            int32 recvBufferSize,											//ϵͳ���ջ����С
            int32 sendBufferSize,											//ϵͳ���ͻ����С
			int32 cacheRecvBuferSize,										//Buffer�Ľ��ջ����С
			int32 cacheSendBufferSize										//Buffer�ķ��ͻ����С
            )
        {
            auto cb = std::bind(
                &TcpClient::connect, 
                this, 
                name,
                serverAddr,
                connectFailedCallback, 
                connectSuccessCallback,
                connectionCallback, 
                messageCallback, 
                writeCompleteCallback, 
                timeoutSecond,
                recvBufferSize, 
                sendBufferSize,
				cacheRecvBuferSize,
				cacheSendBufferSize);
            loop_->runAfter( cb, delaySecond * 1000 );
        }

        void TcpClient::connectComplete( const ConnectorPtr& connector, int sockfd )
        {
            loop_->assertInLoopThread();
            //MDLog( "TcpClient connectComplete[%s].", connector->name().c_str() );
            ConnectorMap::iterator p_it = connectors_.find( connector->name() );
            assert( p_it != connectors_.end() );
            std::unique_ptr<ConnectorContext> context = std::move(p_it->second);
            size_t n = connectors_.erase( connector->name() );
            assert( n == 1 );
            
            if (-1 == sockfd)
            {
                //����ʧ��
                context->connectFailedCallback_();
            }
            else
            {
                char buf[64];
                STRING_FMT( buf, sizeof buf, "-connection#%d", nextConnectionId_++ );
                InetAddress peerAddr( sockets::getPeerAddr( sockfd ) );
                std::string connName = context->name_ + buf;
                //MDLog( "TcpClient, new Connection[%s] to[%s]", connName.c_str(), peerAddr.toIpPort().c_str() );
                InetAddress localAddr( sockets::getLocalAddr( sockfd ) );
                //���̻߳���ֱ��ʹ��tcpServer���ڵ�loop
                TcpConnectionPtr conn( new TcpConnection( loop_, connName, sockfd, localAddr, peerAddr, context->recvBufferSize_, context->sendBufferSize_, context->cacheRecvBufferSize_, context->cacheSendBufferSize_ ) );
                connections_[connName] = conn;
                conn->setConnectionCallback( context->connectionCallback_ );
                conn->setMessageCallback( context->messageCallback_ );
                conn->setWriteCompleteCallback( context->writeCompleteCallback_ );

                //���ӹر�ʱ��Ҫ֪ͨ������TcpServer
                conn->setCloseCallback( std::bind( &TcpClient::removeConnection, this, std::placeholders::_1 ) );
                
                //֪ͨ�û������ӳɹ�
                context->connectSuccessCallback_( conn ); 

                loop_->runInLoop( std::bind( &TcpConnection::connectEstablished, conn ) );
            }
        }

        void TcpClient::removeConnection( const TcpConnectionPtr& conn )
        {
            loop_->assertInLoopThread();
            //MDLog( "TcpClient removeConnection[%s].", conn->name().c_str() );
            size_t n = connections_.erase( conn->name() );
            assert( n == 1 );
            loop_->queueInLoop( std::bind( &TcpConnection::connectDestroyed, conn ) );
        }
	}
}