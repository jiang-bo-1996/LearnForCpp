#include <thread>
#include <queue>
#include <mutex>
#include <iostream>
#include <condition_variable>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>


class threadpool{
 public:
  typedef boost::function<void ()> Functor;

  threadpool()
    : running_(false),
      threadsNum_(0)
  {
  }

  void setThreadsnum(int Threadsnum){
    threadsNum_ = Threadsnum;
  }
  
  void start(){
    assert(!running_);
    running_ = true;
    for(int i = 0; i < threadsNum_; ++ i){
      threads_.push_back(new std::thread(
                         boost::bind(&threadpool::threadFunc, this)));
    }
  }

  ~threadpool(){
    stop();
    std::cout<<"destructor"<<std::endl;
  }

  void push(const Functor &cb){
    if(threads_.empty()){
        cb();
    }else{
      std::cout<<"push..."<<std::endl;
      std::unique_lock<std::mutex> lock(mutex_);
      tasks_.push(cb);
      cond_.notify_one();
    }
  }

  void stop(){
      assert(running_);
      running_ = false;
      for_each(threads_.begin(), 
               threads_.end(),
               boost::bind(&std::thread::join, _1));
  }

  Functor take(){
    std::unique_lock<std::mutex> lock(mutex_);
    while(tasks_.empty() && running_){
      cond_.wait(lock);
    }

    Functor task;
    if(!tasks_.empty()){
      std::cout<<"take successfully"<<std::endl;
      task = tasks_.front();
      tasks_.pop();
    }
    return task;
  }

  void threadFunc(){
    while(running_){
      Functor task(take());
      std::cout<<static_cast<bool>(task)<<std::endl;
      if(!task){
        task();
      }
    }

  }
 private:
  bool running_;
  int threadsNum_;
  boost::ptr_vector<std::thread> threads_;
  std::queue<Functor> tasks_;
  std::mutex mutex_;
  std::condition_variable cond_;
};