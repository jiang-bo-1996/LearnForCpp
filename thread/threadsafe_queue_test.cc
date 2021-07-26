#pragma once


#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <thread>
#include <condition_variable>
#include <iostream>
#include "../timestamp/TimeStamp.h"
#include <queue>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/make_shared.hpp>
#include <mutex>

template <typename T>
class threadsafe_queue_version_2 : boost::noncopyable
{
private:
  struct node{
    boost::shared_ptr<T> data;
    std::unique_ptr<node> next;
  };

  std::mutex head_mutex_;
  std::unique_ptr<node> head_;
  std::mutex tail_mutex_;
  node *tail_;
  std::condition_variable cond_;

  node* get_tail(){
    std::lock_guard<std::mutex> lock(tail_mutex_);
    return tail_;
  }

  std::unique_ptr<node> pop_head(){
    std::unique_ptr<node> old_head = std::move(head_);
    head_ = std::move(old_head->next);
    return old_head;
  }

  std::unique_lock<std::mutex> wait_for_data(){
    std::unique_lock<std::mutex> lock(head_mutex_);
    cond_.wait(lock, [&]{ return head_.get() != get_tail(); });
    return std::move(lock);
  }

  std::unique_ptr<node> wait_pop_head(){
    std::unique_lock<std::mutex> lock(wait_for_data());
    return pop_head();
  }

/*
  boost::scoped_ptr<node> pop_head(){
    std::lock_guard<std::mutex> lock(head_mutex_);
    if(head.get() == get_tail()){
      return nullptr;
    }
    boost::scoped_ptr<node> old_head = std::move(head_);
    head_ = std::move(old_head->next);
    return old_head;
  }
*/
  std::unique_ptr<node> wait_pop_head(T &value){
    std::unique_lock<std::mutex> lock(wait_for_data());
    value = std::move(*head_->data);
    return pop_head();
  }

  std::unique_ptr<node> try_pop_head(){
    std::lock_guard<std::mutex> lock(head_mutex_);
    if(head_.get() == get_tail()){
      return std::unique_ptr<node>();
    }
    return pop_head();
  }

  std::unique_ptr<node> try_pop_head(T &value){
    std::lock_guard<std::mutex> lock(head_mutex_);
    if(head_.get() == get_tail()){
      return std::unique_ptr<node>();
    }
    value = std::move(*head_->data);
    return pop_head();
  }
public:
  threadsafe_queue_version_2()
    : head_(new node), tail_(head_.get())
  { }
/*
  boost::shared_ptr<T> try_pop(){
    boost::scoped_ptr<node> old_head = pop_head();
    return old_head ? old_head->data : boost::shared_ptr<T>();
  }
*/

  boost::shared_ptr<T> wait_and_pop(){
    std::unique_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
  }

  void push(T new_value){
    boost::shared_ptr<T> new_data(
    boost::make_shared<T>(std::move(new_value)));
    std::unique_ptr<node> p(new node);
    node *const new_tail = p.get();
    {
      std::lock_guard<std::mutex> lock(tail_mutex_);
      tail_->data = new_data;
      tail_->next = std::move(p);
      tail_ = new_tail;
    }
    cond_.notify_one();
  }
  void wait_and_pop(T &value){
    std::unique_ptr<node> const old_head = wait_pop_head(value);
  }

  boost::shared_ptr<T> try_pop(){
    std::unique_ptr<node> old_head = try_pop_head();
    return old_head ? old_head->data : boost::shared_ptr<T>();
  }

  bool empty(){
    std::lock_guard<std::mutex> lock(head_mutex_);
    return (head_.get() == get_tail());
  }
};

using namespace jiangbo;


threadsafe_queue_version_2<int> queue_;

void print(){

}

void work1(){
  for(int i = 0; i < 1e7; ++ i){
    queue_.push(i);
  }
}

void work2(){
  for(int i = 0;i < 1e7; ++ i){
    boost::shared_ptr<int> item = queue_.wait_and_pop();
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