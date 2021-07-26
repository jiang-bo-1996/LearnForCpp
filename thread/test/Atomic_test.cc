#include <iostream>
#include <atomic>
#include <thread>
#include "../../timestamp/TimeStamp.h"

class AtomicInt64{
public:
  AtomicInt64(): num_(0){};
  void increment(){
    ++ num_;
  }

  int getnum(){
    return num_;
  }
private:
  std::atomic_int64_t num_;
};

AtomicInt64 atoInt;

void work(){
  for(int i = 0; i < 1e9; ++ i){
   atoInt.increment();
  }
}

int main(){
 jiangbo::Timestamp begin(jiangbo::Timestamp::now());
 std::thread thread_1(work);
 std::thread thread_2(work);
 thread_1.join();
 thread_2.join();
 jiangbo::Timestamp end(jiangbo::Timestamp::now());
 std::cout << "The atomic variable is " << atoInt.getnum() << std::endl;
 std::cout << "It has cost " << jiangbo::timeDiffence(begin, end) << "s.";
 return 0;
}