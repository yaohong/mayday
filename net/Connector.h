#ifndef _Connector_h__
#define _Connector_h__

#include "InetAddress.h"
#include "Callbacks.h"
#include <functional>
#include <memory>
namespace mayday
{
	namespace net
	{
		//static const int kNormalSpaceSecond = 3;
		static const int kNormalTimeoutSecond = 3;
		class NetworkLoop;
		class Channel;
		class Connector : public std::enable_shared_from_this<Connector>
		{
		public:
			typedef std::function<void(const ConnectorPtr& connector,  int sockfd)> ConnectCompleteCallback;
			Connector(NetworkLoop* loop, const std::string &name, const InetAddress& serverAddr, int timeoutSecond = kNormalTimeoutSecond);
			~Connector();

			void setConnectCompleteCallback(const ConnectCompleteCallback& cb)
			{
				connectCompleteCallback_ = cb;
			}

			const std::string & name() const { return name_; };


			void start();  //
			void stop();  //

			const InetAddress& serverAddress() const { return serverAddr_; }
		private:
			enum States { kDisconnected, kConnecting, kConnected };
			void setState(States s) { state_ = s; }
			void startInLoop();
			void stopInLoop();
			void connect();
			void connecting(int sockfd);
			//void retry(int sockfd);
			int removeAndResetChannel();
			void resetChannel();

			void connectSuccess(int fd);
			void connectFailed(int fd);
		private:
			void handleWrite();
			void handleError();
			void connectTimeoutCheck();
            static void _connectTimeoutCheck( std::weak_ptr<Connector> );
		private:
			NetworkLoop* loop_;
			InetAddress serverAddr_;
			bool connect_;	//
			States state_;  //
			std::unique_ptr<Channel> channel_;
			ConnectCompleteCallback connectCompleteCallback_;

			int timeoutSecond_;
			std::string name_;
		};
	}
}

















#endif