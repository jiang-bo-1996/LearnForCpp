#pragma once

#include "InetAddress.h"
#include "TimerId.h"
#include "copyable.h"

#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace jiangbo{
class Channel;
class EventLoop;
class Connector : boost::noncopyable
{
public:
  typedef boost::function<void (int sockfd)> NewConnectionCallback;

  Connector(EventLoop *loop, const InetAddress &serverAddr);
  ~Connector();

  void setNewConnectionCallBack(const NewConnectionCallback &cb)
  {newConnectionCallBack_ = cb;}

  void start();
  void restart();
  void stop();

  const InetAddress &serverAddress() const {return serverAddr_;}
private:
  enum States {kDisconnected, kConnecting, kConnected };
  static const int kMaxRetryDelayMs = 30 * 1000;
  static const int kInitRetryDalayMs = 500;

  void setState(States s){ state_ = s;}
  void startInLoop();
  void connect();
  void connecting(int sockfd);
  void handleWrite();
  void handleError();
  void retry(int retry);
  int removeAndResetChannel();
  void resetChannel();

  EventLoop *loop_;
  InetAddress serverAddr_;
  bool connect_;
  States state_;
  boost::scoped_ptr<Channel> channel_;
  NewConnectionCallback newConnectionCallBack_;
  int retryDelayMs_;
  TimerId timerId_;
};

typedef boost::shared_ptr<Connector> ConnectorPtr;
}