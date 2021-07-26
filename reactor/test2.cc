#include "Channel.h"
#include "EventLoop.h"

#include <stdio.h>
#include <sys/timerfd.h>

jiangbo::EventLoop* g_loop;

void timeout(jiangbo::Timestamp receiveTime)
{
  printf("%s Timeout!\n", receiveTime.toFormattedString().c_str());
  g_loop->quit();
}

int main()
{
  printf("%s started\n", jiangbo::Timestamp::now().toFormattedString().c_str());
  jiangbo::EventLoop loop;
  g_loop = &loop;

  int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  jiangbo::Channel channel(&loop, timerfd);
  channel.setreadCallback(timeout);
  channel.enableReading();

  struct itimerspec howlong;
  bzero(&howlong, sizeof howlong);
  howlong.it_value.tv_sec = 5;
  ::timerfd_settime(timerfd, 0, &howlong, NULL);

  loop.loop();

  ::close(timerfd);
}