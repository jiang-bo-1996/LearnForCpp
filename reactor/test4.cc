  
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <stdio.h>
#include "TimerId.h"

void runInThread()
{
  printf("runInThread(): pid = %d, tid = %d\n",
         getpid(), jiangbo::CurrentThread::tid());
}

int main()
{
  printf("main(): pid = %d, tid = %d\n",
         getpid(), jiangbo::CurrentThread::tid());

  jiangbo::EventLoopThread loopThread;
  jiangbo::EventLoop* loop = loopThread.startLoop();
  loop->runInLoop(runInThread);
  sleep(1);
  loop->runAfter(2, runInThread);
  sleep(3);
  loop->quit();

  printf("exit main().\n");
}