#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

using namespace jiangbo;

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseloop)
  : baseLoop_(baseloop),
    started_(false),
    next_(0),
    numThreads_(0)
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::start(){
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    threads_.reserve(numThreads_);
    for(int i = 0; i < numThreads_; ++ i){
        EventLoopThread *thread_ = new EventLoopThread;
        threads_.push_back(thread_);
        loops_.push_back(thread_->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop(){
    baseLoop_->assertInLoopThread();
    EventLoop *loop = baseLoop_;

    if(!loops_.empty()){
        loop = loops_[next_];
        ++ next_;
        if(static_cast<size_t>(next_) >= loops_.size()){
            next_ = 0;
        }
    }
    return loop;
}