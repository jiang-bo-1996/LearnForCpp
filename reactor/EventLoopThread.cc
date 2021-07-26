#include "EventLoopThread.h"

#include <boost/bind.hpp>

using namespace jiangbo;

EventLoopThread::EventLoopThread()
  : loop_(nullptr),
    existing_(false),
    thread_(boost::bind(&EventLoopThread::ThreadFunc, this))
{

}

EventLoopThread::~EventLoopThread(){
    existing_ = true;
    thread_.join();
    loop_->quit();
}

EventLoop *EventLoopThread::startLoop(){
    assert(!thread_.started());
    thread_.start();

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == NULL)
        {
            cond_.wait(lock);
        }
    }
    
    return loop_;
}

void EventLoopThread::ThreadFunc(){
    EventLoop loop;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop_->loop();
}