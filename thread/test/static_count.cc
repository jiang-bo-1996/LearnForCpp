#include <mutex>
#include <thread>
#include <iostream>
#include "../../timestamp/TimeStamp.h"
//#define STATIC_TEST

class Counter{
private:
  int64_t counter_;
#ifndef STATIC_TEST
  std::mutex mutex_;
#else
  static std::mutex mutex_;
#endif

public:
  Counter()
    : counter_(0)
  {  };

  void add(){
    std::lock_guard<std::mutex> lock(mutex_);
    ++ counter_;
  };
};

Counter cnt_;

void work(){
  for(int i = 0; i < 1e9; ++ i){
    cnt_.add();
  }
}

int main(){
  jiangbo::Timestamp first = jiangbo::Timestamp::now();
  std::thread thread1(work);
  std::thread thread2(work);
  thread1.join();
  thread2.join();
  jiangbo::Timestamp second = jiangbo::Timestamp::now();
  double diff = jiangbo::timeDiffence(second, first);
  std::cout<<"the thread id "<<pthread_self()<<" cost "<<diff<<"s."<<std::endl;
  return 0;
}