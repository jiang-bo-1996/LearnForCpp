#pragma once
#include <boost/noncopyable.hpp>
#include <mutex>

#include "TcpConnection.h"
#include "CallBack.h"

namespace jiangbo
{

class Connector;
typedef boost::shared_ptr<Connector> ConnectorPtr;

class TcpClient : boost::noncopyable
{
 public:
  TcpClient(EventLoop* loop,
            const InetAddress& serverAddr);
  ~TcpClient(); 

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const
  {
    std::lock_guard<std::mutex> lock(mutex_);
    return connection_;
  }

  bool retry() const;
  void enableRetry() { retry_ = true; }


  void setConnectionCallback(const ConnectionCallBack& cb)
  { connectionCallback_ = cb; }

  void setMessageCallback(const MessageCallBack& cb)
  { messageCallback_ = cb; }

  void setWriteCompleteCallback(const WriteCompeleteCallback& cb)
  { writeCompleteCallback_ = cb; }

 private:
  void newConnection(int sockfd);
  void removeConnection(const TcpConnectionPtr& conn);

  EventLoop* loop_;
  ConnectorPtr connector_;
  ConnectionCallBack connectionCallback_;
  MessageCallBack messageCallback_;
  WriteCompeleteCallback writeCompleteCallback_;
  bool retry_;   
  bool connect_; 
  int nextConnId_;
  mutable std::mutex mutex_;
  TcpConnectionPtr connection_;
};

}