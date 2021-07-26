#pragma once
#include "InetAddress.h"
#include "CallBack.h"
#include "Buffer.h"
#include "../timestamp/TimeStamp.h"

#include <string>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/any.hpp>//
namespace jiangbo
{
class EventLoop;
class Socket;
class Channel;

class TcpConnection : boost::noncopyable, 
                      public boost::enable_shared_from_this<TcpConnection>
{
 public:
  TcpConnection(EventLoop *loop,
                const std::string &name,
                int sockfd,
                const InetAddress &localAddr,
                const InetAddress &peerAddr);
  ~TcpConnection();

  EventLoop *getLoop() const {return loop_;}
  const std::string &name() const { return name_;}
  const InetAddress &localAddress() {return localAddr_; }
  const InetAddress &peerAddress() { return peerAddr_; }

  bool connected() const { return state_ == kConnected; }

  void setConnectionCallBack(const ConnectionCallBack &cb)
  { connectionCallback_ = cb; }
  
  void setMessageConnectionCallBack(const MessageCallBack &cb)
  { messageCallback_ = cb; }
  
  void setWriteCompeleteCallback(const WriteCompeleteCallback &cb){
    writeCompleteCallback_ = cb;
  }

  void setCloseCallBack(const CloseCallBack &cb)
  {
    closeCallback_ = cb;
  }
  void connectEstablished();
  
  void connectDestoryed();

  void send(const std::string &message);

  void shutdown();

  void setTcpNoDelay(bool on);

 private:
  enum stateE { kConnecting, kConnected, kDisconnecting, kDisConnected};

  void setState(stateE s){ state_ = s;}
  void handleRead(Timestamp receiveTime);
  void handleWrite();
  void handleClose();
  void handleError();
  void shutdownInLoop();
  void sendInLoop(const std::string &message);
  
  EventLoop* loop_;
  stateE state_;
  std::string name_;
  boost::scoped_ptr<Socket> socket_;
  boost::scoped_ptr<Channel> channel_;
  InetAddress localAddr_;
  InetAddress peerAddr_;
  ConnectionCallBack connectionCallback_;
  MessageCallBack messageCallback_;
  WriteCompeleteCallback writeCompleteCallback_;
  CloseCallBack closeCallback_;
  Buffer inputBuffer_;
  Buffer outputBuffer_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
} // namespace jiangboclass TcpConnection
