#pragma once

#include "../reactor/TcpServer.h"

class EchoServer{
public:
  EchoServer(jiangbo::EventLoop *loop, 
             const jiangbo::InetAddress &listenAddr);

  void start();
private:
  void onConnection(const jiangbo::TcpConnectionPtr & conn);

  void onMessage(const jiangbo::TcpConnectionPtr &conn, 
                 jiangbo::Buffer *buf, 
                 jiangbo::Timestamp time);

  jiangbo::TcpServer server_;
};