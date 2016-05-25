#ifndef _TcpClient_h__
#define _TcpClient_h__

#include "Callbacks.h"
#include "InetAddress.h"

namespace mayday
{
	namespace net
	{
		typedef std::function<void()> ConnectFailedCallback;
		typedef std::function<void(const TcpConnectionPtr &)> ConnectSuccessCallback;

		struct ConnectorContext {
        public:
            const std::string name_;
			ConnectorPtr connectorPtr_;
			ConnectFailedCallback connectFailedCallback_;
			ConnectSuccessCallback connectSuccessCallback_;

			ConnectionCallback connectionCallback_;
			MessageCallback messageCallback_;
			WriteCompleteCallback writeCompleteCallback_;
            int32 recvBufferSize_;
            int32 sendBufferSize_;
            ConnectorContext(
                const std::string &name,
                const ConnectorPtr& connectorPtr,
                const ConnectFailedCallback connectFailedCallback,
                const ConnectSuccessCallback connectSuccessCallback,
                const ConnectionCallback connectionCallback,
                const MessageCallback messageCallback,
                const WriteCompleteCallback writeCompleteCallback,
                int32 recvBufferSize,
                int32 sendBufferSize
                ) 
                : name_( name )
                , connectorPtr_( connectorPtr )
                , connectFailedCallback_( connectFailedCallback )
                , connectSuccessCallback_( connectSuccessCallback )
                , connectionCallback_( connectionCallback )
                , messageCallback_( messageCallback )
                , writeCompleteCallback_( writeCompleteCallback )
                , recvBufferSize_(recvBufferSize)
                , sendBufferSize_( sendBufferSize )
            {
                
            }

            ~ConnectorContext() {
            }

		};

        class NetworkLoop;
        class TcpClient 
		{
		public:
			TcpClient(NetworkLoop *loop);
			~TcpClient();
		public:
			void connect(
                const std::string &name,
				const InetAddress &serverAddr,									//Ŀ��������ĵ�ַ
				const ConnectFailedCallback &connectFailedCallback,				//����ʧ�ܵĻص�����
				const ConnectSuccessCallback &connectSuccessCallback,			//���ӳɹ��Ļص�����	

				const ConnectionCallback &connectionCallback,					//����״̬�仯�Ļص�(ֻ�����ӶϿ�ʱ�����,���ӳɹ��õ���connectSuccessCallback)
				const MessageCallback &messageCallback,							//����Ϣ�Ļص�����
				const WriteCompleteCallback &writeCompleteCallback,				//����д�Ļص�����
				int timeoutSecond,
                int32 recvBufferSize,                                            
                int32 sendBufferSize                                           
				);

            void delayConnect(
                const std::string &name,
                const InetAddress &serverAddr,									//Ŀ��������ĵ�ַ
                const ConnectFailedCallback &connectFailedCallback,				//����ʧ�ܵĻص�����
                const ConnectSuccessCallback &connectSuccessCallback,			//���ӳɹ��Ļص�����	

                const ConnectionCallback &connectionCallback,					//����״̬�仯�Ļص�(ֻ�����ӶϿ�ʱ�����)
                const MessageCallback &messageCallback,							//����Ϣ�Ļص�����
                const WriteCompleteCallback &writeCompleteCallback,				//����д��ϡ�Ļص�����
                int timeoutSecond,
                int delaySecond,
                int32 recvBufferSize,
                int32 sendBufferSize
                );
		private:
            void connectComplete( const ConnectorPtr& connector, int sockfd );

            void removeConnection( const TcpConnectionPtr& conn );
		private:
			typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;			
			typedef std::map<std::string, std::unique_ptr<ConnectorContext> > ConnectorMap;				//�������ӵ�TcpConnection
			ConnectionMap connections_;			//�Ѿ����ӳɹ���connect
			ConnectorMap connectors_;			//�������ӵ�

            int nextConnectorId_;
            int nextConnectionId_;

			NetworkLoop *loop_;
		};
	}
}














#endif