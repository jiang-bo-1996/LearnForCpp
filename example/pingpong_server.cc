#include "../reactor/TcpServer.h"

#include "../thread/Thread.h"
#include "../logging/Logging.h"
#include "../thread/Thread.h"
#include "../reactor/EventLoop.h"
#include "../reactor/InetAddress.h"
#include "../reactor/TcpConnection.h"

#include <boost/bind.hpp>

#include <utility>

#include <mcheck.h>
#include <stdio.h>
#include <unistd.h>

using namespace jiangbo;

void onConnection(const TcpConnectionPtr& conn)
{
  conn->setTcpNoDelay(true);
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
  std::string msg(buf->retrieveAsString());
  conn->send(msg);
}

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    fprintf(stderr, "Usage: server <address> <port> <threads>\n");
  }
  else
  {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    Logger::setLogLevel(Logger::DEBUG);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress listenAddr(ip, port);
    int threadCount = atoi(argv[3]);

    EventLoop loop;

    TcpServer server(&loop, listenAddr);

    server.setConnectionCallBack(onConnection);
    server.setMessageCallBack(onMessage);

    if (threadCount > 1)
    {
      server.setThreadNum(threadCount);
    }

    server.start();

    loop.loop();
  }
}