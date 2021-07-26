#include "echo.h"
#include "../logging/Logging.h"
#include "../reactor/TcpConnection.h"
#include "../reactor/InetAddress.h"

#include <boost/bind.hpp>

EchoServer::EchoServer(jiangbo::EventLoop *loop, 
                       const jiangbo::InetAddress &listenAddr)
  : server_(loop, listenAddr)
{
  server_.setConnectionCallBack(boost::bind(&EchoServer::onConnection, this, _1));
  server_.setMessageCallBack(boost::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start(){
  server_.start();
}

void EchoServer::onConnection(const jiangbo::TcpConnectionPtr & conn){
  LOG_INFO << "EchoServer - " << conn->peerAddress().toHostPort() << " -> "
           << conn->localAddress().toHostPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const jiangbo::TcpConnectionPtr &conn,
                             jiangbo::Buffer *buf,
                             jiangbo::Timestamp time)
{
  std::string msg(buf->retrieveAsString());
  LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
           << "data received at " << time.toString();
  LOG_DEBUG << "the msg data is : " << msg.data();
  conn->send(msg);
}