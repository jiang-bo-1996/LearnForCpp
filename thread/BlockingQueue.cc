#pragma once

#include <boost/noncopyable.hpp>
#include <mutex>
#include <queue>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <thread>
#include <condition_variable>
#include <iostream>
#include <memory>
#include "../timestamp/TimeStamp.h"
namespace jiangbo{
template <typename T>
class BlockingQueue : boost::noncopyable{
public:
  BlockingQueue(){ }

  void put(const T x){
    std::shared_ptr<T> new_data(std::make_shared<T>(x));
    {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(new_data);
    }
    notEmpty_.notify_one();
  }

  std::shared_ptr<T> take(){
    std::unique_lock<std::mutex> lock(mutex_);
    notEmpty_.wait(lock, [&](){ return !queue_.empty();});
    assert(!queue_.empty());
    std::shared_ptr<T> front(queue_.front());
    queue_.pop();
    return front;
  }

  size_t size() const{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }
private:
  mutable std::mutex mutex_;
  std::condition_variable notEmpty_;
  std::queue<std::shared_ptr<T>> queue_;
};
} // namespace jiangbo


using namespace jiangbo;


BlockingQueue<int> queue_;


void work1(){
  for(int i = 0; i < 1e7; ++ i){
    queue_.put(i);
  }
}

void work2(){
  for(int i = 0;i < 1e7; ++ i){
    std::shared_ptr<int> item = queue_.take();
  }
}

int main(){
  jiangbo::Timestamp begin(jiangbo::Timestamp::now());
  std::thread thread1(work1);
  std::thread thread2(work2);
  thread1.join();
  thread2.join();
  jiangbo::Timestamp end(jiangbo::Timestamp::now());
  std::cout << "cost " << jiangbo::timeDiffence(end, begin) << " s;";
  return 0;
}