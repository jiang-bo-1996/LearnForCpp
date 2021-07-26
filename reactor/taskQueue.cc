#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>



class XXX{
public:
    XXX(int a = 10)
      : a_(a)
    { std::cout<<"XXX()";}

    XXX(const XXX &X){
        std::cout<<"XXX(const XXX &X)"<<std::endl;
        if(this != &X){
            a_ = X.a_;
        }
    }

    XXX& operator=(const XXX &rhs){
        std::cout<<"operator(const XXX &rhs)"<<std::endl;
        a_ = rhs.a_;
        return *this;
    }
    int a(){
        return a_;
    }
private:
    int a_;
};

class TaskQueue{
public:
    typedef boost::function<void ()> function;

    TaskQueue(){}

    void put(function func){
        taskqueue_.push_back(func);
        cv_.notify_one();
    }

    function take(){
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&](){ return !taskqueue_.empty(); });
        return taskqueue_.back();
    }

private:
    std::vector<function> taskqueue_;
    std::mutex mutex_;
    std::condition_variable cv_;
};