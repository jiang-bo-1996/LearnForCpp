#include <exception>
#include <memory>
#include <mutex>
#include <stack>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

struct empty_stack: std::exception
{
    const char *what() const throw(){
        return "empty stack!";
    };
};

template <typename T>
class threadsafe_stack
{
private:
  std::stack<T> data_;
  mutable std::mutex mutex_;

public:
  threadsafe_stack()
    : data(std::stack<T>()){ }
  
  threadsafe_stack(const threadsafe_stack &other){
    std::lock_guard<std::mutex> lock(other.mutex_);
    date_ = other.data_;
  }

  threadsafe_stack &operator= (const threadsafe_stack&) = delete;

  void push(T new_value){
    std::lock_guard<std::mutex> lock(mutex_);
    data_.push(new_value);
  }

  boost::shared_ptr<T> pop(){
    std::lock_guard<std::mutex> lock(mutex_);
    if(data_.empty()){
      throw empty_stack();
    }

    boost::shared_ptr<T> const res(boost::make_shared<T>(data_.top()));

    data.pop();
    return res;
  }

  void pop(T &value){
    std::lock_guard<std::mutex> lock(mutex_);
    if(data.empty()){
        throw empty_stack();
    }

    value = data_.top();
    data_.pop();
  }

  bool empty() const{
      std::lock_guard<std::mutex> lock(mutex_);
      return data.empty();
  }
};