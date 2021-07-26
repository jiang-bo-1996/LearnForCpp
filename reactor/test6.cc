#include "Connector.h"
#include "EventLoop.h"
#include "../logging/Logging.h"

#include <stdio.h>

jiangbo::EventLoop* g_loop;

void connectCallback(int sockfd)
{
  printf("connected.\n");
  g_loop->quit();
}

int main(int argc, char* argv[])
{
  jiangbo::Logger::setLogLevel(jiangbo::Logger::TRACE);
  jiangbo::EventLoop loop;
  g_loop = &loop;
  jiangbo::InetAddress addr("127.0.0.1", 55555);
  jiangbo::ConnectorPtr connector(new jiangbo::Connector(&loop, addr));
  connector->setNewConnectionCallBack(connectCallback);
  connector->start();

  loop.loop();
}