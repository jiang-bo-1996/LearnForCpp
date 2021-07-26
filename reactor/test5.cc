#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socketops.h"
#include <stdio.h>

void newConnection(int sockfd, const jiangbo::InetAddress& peerAddr)
{
  printf("newConnection(): accepted a new connection from %s\n",
         peerAddr.toHostPort().c_str());
  ::write(sockfd, "How are you?\n", 13);
  jiangbo::socketops::closefd(sockfd);
}

int main()
{
  printf("main(): pid = %d\n", getpid());

  jiangbo::InetAddress listenAddr(9981);
  jiangbo::EventLoop loop;

  jiangbo::Acceptor acceptor(&loop, listenAddr);
  acceptor.setNewConnectionCallBack(newConnection);
  acceptor.listen();

  loop.loop();
}