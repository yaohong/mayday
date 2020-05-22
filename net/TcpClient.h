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
			int32 cacheRecvBufferSize_;
			int32 cacheSendBufferSize_;
            ConnectorContext(
                const std::string &name,
                const ConnectorPtr& connectorPtr,
                const ConnectFailedCallback connectFailedCallback,
                const ConnectSuccessCallback connectSuccessCallback,
                const ConnectionCallback connectionCallback,
                const MessageCallback messageCallback,
                const WriteCompleteCallback writeCompleteCallback,
                int32 recvBufferSize,
                int32 sendBufferSize,
				int32 cacheRecvBufferSize,
				int32 cacheSendBufferSize
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
				, cacheRecvBufferSize_(cacheRecvBufferSize)
				, cacheSendBufferSize_(cacheSendBufferSize)
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
				const InetAddress &serverAddr,									//目标服务器的地址
				const ConnectFailedCallback &connectFailedCallback,				//连接失败的回调函数
				const ConnectSuccessCallback &connectSuccessCallback,			//连接成功的回调函数	

				const ConnectionCallback &connectionCallback,					//连接状态变化的回调(只有连接断开时会调用,连接成功用的是connectSuccessCallback)
				const MessageCallback &messageCallback,							//收消息的回调函数
				const WriteCompleteCallback &writeCompleteCallback,				//可以写的回调函数
				int timeoutSecond,
                int32 recvBufferSize,                                            
                int32 sendBufferSize,
				int32 cacheRecvBuferSize,
				int32 cacheSendBufferSize
				);

            void delayConnect(
                const std::string &name,
                const InetAddress &serverAddr,									//目标服务器的地址
                const ConnectFailedCallback &connectFailedCallback,				//连接失败的回调函数
                const ConnectSuccessCallback &connectSuccessCallback,			//连接成功的回调函数	

                const ConnectionCallback &connectionCallback,					//连接状态变化的回调(只有连接断开时会调用)
                const MessageCallback &messageCallback,							//收消息的回调函数
                const WriteCompleteCallback &writeCompleteCallback,				//可以写调稀的回调函数
                int timeoutSecond,
                int delaySecond,
                int32 recvBufferSize,
                int32 sendBufferSize,
				int32 cacheRecvBuferSize,
				int32 cacheSendBufferSize
                );
		private:
            void connectComplete( const ConnectorPtr& connector, int sockfd );

            void removeConnection( const TcpConnectionPtr& conn );
		private:
			typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;			
			typedef std::map<std::string, std::unique_ptr<ConnectorContext> > ConnectorMap;				//正在连接的TcpConnection
			ConnectionMap connections_;			//已经连接成功的connect
			ConnectorMap connectors_;			//正在连接的

            int nextConnectorId_;
            int nextConnectionId_;

			NetworkLoop *loop_;
		};
	}
}














#endif