#pragma once
#include "../thread/Thread.h"
#include "EventLoop.h"


#include <boost/noncopyable.hpp>
#include <mutex>
#include <condition_variable>


namespace jiangbo{
class EventLoop;
class EventLoopThread : boost::noncopyable{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop *startLoop();
private:
    void ThreadFunc();
    
    EventLoop *loop_;
    bool existing_;
    Thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
};
}