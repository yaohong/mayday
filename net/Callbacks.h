#ifndef _Callbacks_h__
#define _Callbacks_h__

#include "../base/Define.h"

namespace mayday
{
    namespace net
    {
        class TcpConnection;
		class Connector;
        typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
		typedef std::shared_ptr<Connector> ConnectorPtr;
        class Buffer;
        typedef std::function<void( const TcpConnectionPtr& )> ConnectionCallback;
        typedef std::function<void( const TcpConnectionPtr& )> CloseCallback;
        typedef std::function<void( const TcpConnectionPtr& )> WriteCompleteCallback;
        //typedef std::function<void( const TcpConnectionPtr&, size_t )> HighWaterMarkCallback;
        typedef std::function<void( const TcpConnectionPtr&, Buffer* )> MessageCallback;


        typedef std::function<int( const char* ,int)> PacketHeadParseCallback;
    }
}






#endif