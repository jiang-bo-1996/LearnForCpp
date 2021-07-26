#include "echo.h"
#include "../reactor/EventLoop.h"
#include "../logging/Logging.h"
#include "../reactor/InetAddress.h"
#include <unistd.h>

int main(){
  LOG_INFO << "pid = " << getpid();
  jiangbo::EventLoop loop;
  jiangbo::InetAddress listenAddr(9981);
  EchoServer server(&loop, listenAddr);
  server.start();
  loop.loop();
}