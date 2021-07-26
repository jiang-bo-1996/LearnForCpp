#include "TcpServer.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <stdio.h>

void onConnection(const jiangbo::TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    printf("onConnection(): tid=%d new connection [%s] from %s\n",
           jiangbo::CurrentThread::tid(),
           conn->name().c_str(),
           conn->peerAddress().toHostPort().c_str());
  }
  else
  {
    printf("onConnection(): tid=%d connection [%s] is down\n",
           jiangbo::CurrentThread::tid(),
           conn->name().c_str());
  }
}

void onMessage(const jiangbo::TcpConnectionPtr& conn,
               jiangbo::Buffer* buf,
               jiangbo::Timestamp receiveTime)
{
  printf("onMessage(): tid=%d received %zd bytes from connection [%s] at %s\n",
         jiangbo::CurrentThread::tid(),
         buf->readableBytes(),
         conn->name().c_str(),
         receiveTime.toFormattedString().c_str());

  conn->send(buf->retrieveAsString());
}

int main(int argc, char* argv[])
{
  printf("main(): pid = %d\n", getpid());

  jiangbo::InetAddress listenAddr(9981);
  jiangbo::EventLoop loop;

  jiangbo::TcpServer server(&loop, listenAddr);
  server.setConnectionCallBack(onConnection);
  server.setMessageCallBack(onMessage);
  if (argc > 1) {
    server.setThreadNum(atoi(argv[1]));
  }
  server.start();

  loop.loop();
}