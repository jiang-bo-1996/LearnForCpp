#include "TcpClient.h"
#include "EventLoop.h"
#include "TimerId.h"
#include "TcpConnection.h"

jiangbo::TcpClient *g_client;

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

void timeout(){
  g_client->disconnect();
}

int main(){
    printf("main(): pid = %6d\n", gettid());

    jiangbo::InetAddress serverAddr("106.13.3.180", 9981);
    jiangbo::EventLoop loop;

    jiangbo::TcpClient client(&loop, serverAddr);
    g_client = &client;
    client.setConnectionCallback(onConnection);
    client.setMessageCallback(onMessage);
    client.connect();
    loop.runAfter(3.0, timeout);
    loop.loop();
    return 0;
}
