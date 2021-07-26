#pragma once
#include "../timestamp/TimeStamp.h"

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
namespace jiangbo
{
class TcpConnection;
class Buffer;

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
class TcpConnection;
typedef boost::function<void ()> TimerCallBack;
typedef boost::function<void (const TcpConnectionPtr &)> ConnectionCallBack;
typedef boost::function<void (const TcpConnectionPtr &,
                              Buffer *data, 
                              Timestamp)> MessageCallBack;
typedef boost::function<void (const TcpConnectionPtr &)> CloseCallBack;

typedef boost::function<void (const TcpConnectionPtr &)> WriteCompeleteCallback;
} // namespace jiangbo