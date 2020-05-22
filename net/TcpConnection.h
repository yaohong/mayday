#ifndef _TcpConnection_h__
#define _TcpConnection_h__

#include "InetAddress.h"
#include "Callbacks.h"
#include "Buffer.h"
namespace mayday
{
    namespace net
    {
        class NetworkLoop;
        class Channel;
        class Socket;
        class TcpConnection : public std::enable_shared_from_this<TcpConnection>
        {
            friend class TcpServer;
            friend class TcpClient;
        public:
            TcpConnection( 
                NetworkLoop *loop, 
                const std::string &name, 
                int sockfd, 
                const InetAddress &localAddr, 
                const InetAddress &peerAddr,
                int32 sysRecvSize, 
                int32 sysSendSize,
				int32 cacheRecvSize,
				int32 cacheSendSize
                );
            ~TcpConnection();
            NetworkLoop* getLoop() const { return loop_; }
            const std::string &name() const { return name_; };
            const InetAddress& localAddress() const { return localAddr_; }
            const InetAddress& peerAddress() const { return peerAddr_; }
            bool connected() const { return state_ == kConnected; }
            bool disconnected() const { return state_ == kDisconnected; }

            void send( const void *data, int len );
            void delaySend(const void *data, int len);


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

            void setCloseCallback( const CloseCallback& cb )
            {
                closeCallback_ = cb;
            }

            void forceClose( );

            void setContext( void* context )
            {
                context_ = context;
            }

            void* getContext() const
            {
                return context_;
            }


            void setSendParam( bool isDelay, int delayTime = 0, int maxBuf = 0 ) 
            { 
                isDelaySend_ = true; 
                sendDelayTime_ = delayTime;
            };
        private:
            void connectEstablished( );
            void connectDestroyed( );
        private:
            void forceCloseInLoop();
            enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };
            void handleRead();
            void handleWrite();
            void handleClose();
            void handleError();
            void setState( StateE s ) { state_ = s; }

            static void _handDelaySend( std::weak_ptr<TcpConnection> ptr );
            void handDelaySend();
        private:
            const std::string name_;
            StateE state_;
            const InetAddress localAddr_;
            const InetAddress peerAddr_;
            NetworkLoop *loop_;

            std::unique_ptr<Socket> socket_;
            std::unique_ptr<Channel> channel_;

            ConnectionCallback connectionCallback_;
            MessageCallback messageCallback_;
            WriteCompleteCallback writeCompleteCallback_;
            CloseCallback closeCallback_;
            
            Buffer recvBuffer_;
            Buffer sendBuffer_;
            void *context_;
			bool destroyed_;


            bool isDelaySend_;          //是否延时发送
            int32 sendDelayTime_;       //延时的时间10-1000毫秒
            int32 maxSendCacheSize_;     //最大的发送缓存

            bool isStartDelayTask_;     //是否开始延时任务
            uint64 curDelayTaskId_;     //当前任务ID
            uint64 delayTaskIdAlloc_;   //延时任务ID分配器

        };
    }
}

#endif