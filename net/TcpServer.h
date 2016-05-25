#ifndef _TcpServer_h__
#define _TcpServer_h__
#include "Callbacks.h"
#include "Acceptor.h"
namespace mayday
{
    namespace net
    {
        class NetworkLoop;
        class InetAddress;
        class TcpServer
        {
            static const int recvBufferSize = 8 * 1024;           //���ջ���Ϊ8K
            static const int sendBuffSize = 16 * 1024;           //���ͻ���Ϊ16K
        public:
            TcpServer( NetworkLoop *loop, const std::string &name, const InetAddress &listenAddr, int32 recvSize = recvBufferSize, int32 sendSize = sendBuffSize );
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

            NetworkLoop *loop_;
            Acceptor acceptor_;

            //��tcpserver������connect����Ļص�����
            ConnectionCallback connectionCallback_;             //�µ����ӻ��߶Ͽ����ӣ�����״̬������
            MessageCallback messageCallback_;                   //�յ���������
            WriteCompleteCallback writeCompleteCallback_;       //���ݷ������
        };
    }
}
 


#endif 