#pragma once

#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace jiangbo{
class EventLoop;
class EventLoopThread;
class EventLoopThreadPool : boost::noncopyable{
public:
    EventLoopThreadPool(EventLoop *baseloop);

    ~EventLoopThreadPool();

    void setThreadNum(int numThreads){ numThreads_ = numThreads; }

    void start();

    EventLoop *getNextLoop();
private:
    EventLoop *baseLoop_;
    bool started_;
    int numThreads_;
    int next_;
    boost::ptr_vector<EventLoopThread> threads_;
    std::vector<EventLoop *> loops_;
};
}