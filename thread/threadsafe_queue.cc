#include <thread>
#include <queue>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <condition_variable>
#include <mutex>

namespace jiangbo
{
template <typename T>
class threadsafe_queue
{
  std::queue<T> queue_;
  mutable std::mutex mutex_;
  std::condition_variable cond_;
public:

  threadsafe_queue(const threadsafe_queue &other){
    std::lock_guard<std::mutex> lock(other.mutex_);
    queue_ = other.queue_;
  }
  void push(T new_value){
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(new_value);
    cond_.notify_one();
  }

  void wait_and_pop(T &value){
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.wait(lock, [this]{return !queue_.empty();});
    value = queue_.front();
    queue_.pop();
  }

  boost::shread_ptr<T> wait_and_pop(){
    std::unique_lock<std::mutex> lock(mutex_);
    boost::shared_ptr<T> res(make_shared<T>(queue_.front()));
    queue_.pop();
    return res;
  }

  bool try_pop(T &value){
    std::lock_guard<std::mutex> lock(mutex_);
    if(queue_.empty()){
      return false;
    }
    value = queue_.front();
    queue_.pop();
    return true;
  }

  boost::shared_ptr<T> try_pop(){
    std::lock_guard<std::mutex> lock(mutex_);
    if(queue_.empty()){
      return boost::shared_ptr<T>();
    }
    boost::shared_ptr<T> res(make_shared<T>(queue_.front()));
    queue_.pop();
    return res;
  }

  bool empty() const{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
  }
};

template <typename T>
class threadsafer_queue
{
private:
  mutable std::mutex mutex_;
  std::queue<boost::shared_ptr<T>> data_queue;
  std::condition_variable cond_;

public:
  threadsafer_queue()
  { }

  void push(T new_value){
    boost::shared_ptr<T> data(make_shared<T>(new_value));
    std::lock_guard<std::mutex> lock(mutex_);
    data_queue.push(data);
    cond_.notify_one();
  }

  void wait_and_pop(T &value){
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]{ return !data_queue.empty(); });
    value = std::move(*data_queue.front());
    data_queue.pop();
  }

  boost::shared_ptr<T> wait_and_pop(){
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this]{ return !data_queue.empty(); });
    std::shared_ptr<T> res = data_queue.front();
    data_queue.pop();
    return res;
  }

  bool try_pop(T &value){
    std::lock_guard<std::mutex> lock(mutex_);
    if(data_queue.empty()){
      return false;
    }
    value = *data_queue.front();
    data_queue.pop();
    return true;
  }

  boost::shared_ptr<T> try_pop(){
    std::lock_guard<std::mutex> lock(mutex_);
    if(data_queue.empty()){
      return std::shared_ptr<T>();
    }
    std::shared_ptr<T> res = data_queue.front();
    data_queue.pop();
    return res;
  }

  bool empty() const{
    std::lock_guard<std::mutex> lock(mutex_);
    return data_queue.empty();
  }
};

template <typename>
class threadsafe_queue_version_2 : boost::noncopyable
{
private:
  struct node{
    boost::shared_ptr<T> data;
    boost::scoped_ptr<node> next;
  };

  std::mutex head_mutex_;
  boost::scoped_ptr<node> head_;
  std::mutex tail_mutex_;
  node *tail;

  node* get_tail(){
    std::lock_guard<std::mutex> lock(tail_mutex_);
    return tail_;
  }

  boost::scoped_ptr<node> pop_head(){
    boost::scoped_ptr<node> old_head = std::move(head_);
    head_ = std::move(old_head->next);
    return old_head;
  }

  boost::scoped_ptr<std::mutex> wait_for_data(){
    std::unique_lock<std::mutex> lock(head_mutex_);
    cond_.wait(lock, [&]{ return head_.get() != get_tail(); });
    return std::move(lock);
  }

  boost::scoped_ptr<node> wait_pop_head(){
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
  boost::scoped_ptr<node> wait_pop_head(T &value){
    std::unique_lock<std::mutex> lock(wait_for_data());
    value = std::move(*head_->data);
    return pop_head();
  }

  boost::scoped_ptr<node> try_pop_head(){
    std::lock_guard<std::mutex> lock(head_mutex_);
    if(head_.get() == get_tail()){
      return boost::scoped_ptr<node>();
    }
    return pop_head();
  }

  boost::scoped_ptr<node> try_pop_head(T &value){
    std::lock_guard<std::mutex> lock(head_mutex_);
    if(head.get() == get_tail()){
      return boost::scoped_ptr<node>();
    }
    value = std::move(*head->data);
    return pop_head();
  }
public:
  threadsafe_queue_version_2()
    : head_(new node), tail_(head.get())
  { }
/*
  boost::shared_ptr<T> try_pop(){
    boost::scoped_ptr<node> old_head = pop_head();
    return old_head ? old_head->data : boost::shared_ptr<T>();
  }
*/

  boost::shared_ptr<T> wait_and_pop(){
    boost::scoped_ptr<node> const old_head = wait_pop_head();
    return old_head->data;
  }

  void push(T new_value){
    boost::shared_ptr<T> new_data(
    boost::make_shared<T>(std::move(new_value)));
    boost::scoped_ptr<node> p(new node);
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
    boost::scoped_ptr<node> const old_head = wait_pop_head(value);
  }

  boost::shared_ptr<T> try_pop(){
    boost::scoped_ptr<node> old_head = try_pop_head();
    return old_head ? old_head->data : boost::shared_ptr<T>();
  }

  bool empty(){
    std::lock_guard<std::mutex> lock(head_mutex_);
    return (head.get() == get_tail());
  }
};

} // namespace jiangbo
