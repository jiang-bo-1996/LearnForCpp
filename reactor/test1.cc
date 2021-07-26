#include "EventLoop.h"
#include "../thread/Thread.h"
#include <stdio.h>

void threadFunc()
{
  printf("threadFunc(): pid = %d, tid = %d\n",
         getpid(), jiangbo::CurrentThread::tid());

  jiangbo::EventLoop loop;
  loop.loop();
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), jiangbo::CurrentThread::tid());

  jiangbo::EventLoop loop;

  jiangbo::Thread thread(threadFunc);
  thread.start();

  loop.loop();
  pthread_exit(NULL);
  return 0;
}