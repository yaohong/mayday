#ifndef _TcpServer_h__
#define _TcpServer_h__
#include "Callbacks.h"
#include "Acceptor.h"
#include <functional>
namespace mayday
{
    namespace net
    {
        class NetworkLoop;
        class InetAddress;
        class TcpServer : public std::enable_shared_from_this<TcpServer>
        {
            static const int recvBufferSize = 8 * 1024;           //接收缓存为8K
            static const int sendBuffSize = 16 * 1024;           //发送缓存为16K
        public:
			TcpServer(NetworkLoop *loop, const std::string &name, const InetAddress &listenAddr, int32 recvSize, int32 sendSize, int32 cacheRecvSize, int32 cacheSendSize);
            ~TcpServer();

            void start();

            void setConnectionCallback( const ConnectionCallback& cb )
            {
                connectionCallback_ = cb;
            }

            void setMessageCallback( const MessageCallback& cb )
            {
                messageCallback_ = cb;
            }

            void setWriteCompleteCallback( const WriteCompleteCallback& cb )
            {
                writeCompleteCallback_ = cb;
            }
        private:
            void newConnection( int sockfd, const InetAddress& peerAddr );
            void removeConnection( const TcpConnectionPtr& conn );
            static void _removeConnection( std::weak_ptr<TcpServer> weak, const TcpConnectionPtr& conn );
            void removeConnectionInLoop( const TcpConnectionPtr& conn );
        private:
            typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

            const std::string name_;
            const std::string ipPort_;
            bool started_;
            int nextConnId_;
            ConnectionMap connections_;

            int32 recvBufferSize_;
            int32 sendBuffSize_;

			int32 cacheRecvBufferSize_;
			int32 cacheSendBufferSize_;

            NetworkLoop *loop_;
            Acceptor acceptor_;

            //该tcpserver下所有connect共享的回调函数
            ConnectionCallback connectionCallback_;             //新的连接或者断开连接，根据状态来区分
            MessageCallback messageCallback_;                   //收到完整包了
            WriteCompleteCallback writeCompleteCallback_;       //数据发送完毕
        };
    }
}
 


#endif 