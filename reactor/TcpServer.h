#pragma once
#include "CallBack.h"

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <map>
namespace jiangbo
{
class EventLoop;
class Acceptor;
class InetAddress;
class EventLoopThreadPool;
class TcpServer : boost::noncopyable
{
public:
  TcpServer(EventLoop *loop, const InetAddress &listenaddr);

  ~TcpServer();

  void setThreadNum(int numThreads);

  void setConnectionCallBack(const ConnectionCallBack &cb)
  { connectionCallback_ = cb; }

  void setMessageCallBack(const MessageCallBack &cb)
  { messageCallback_ = cb; }

  void start();

private:
  void newConnection(int sockfd, const InetAddress &peeraddr);

  void removeConnection(const TcpConnectionPtr &conn);

  void removeConnectionInLoop(const TcpConnectionPtr &conn);
 
  typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;
 
  EventLoop *loop_;
  bool started_;
  int nextConnfd_;
  std::string name_;
  boost::scoped_ptr<Acceptor> acceptor_;
  boost::scoped_ptr<EventLoopThreadPool> threadPool_;
  ConnectionCallBack connectionCallback_;
  MessageCallBack messageCallback_;
  ConnectionMap connections_;
};
} // namespace jiangbo