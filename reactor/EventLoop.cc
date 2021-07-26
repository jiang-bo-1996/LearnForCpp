#include "EventLoop.h"
#include "Epoller.h"
#include "Timer.h"
#include "TimerId.h"
#include "TimerQueue.h"
#include "../logging/Logging.h"

#include <sys/eventfd.h>
#include <boost/bind.hpp>
#include <assert.h>
#include <signal.h>

namespace jiangbo{
int eventfd_create(){
    int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(eventfd < 0){
        abort();
    }
    return eventfd;
}
class IgnoreSigPipe{
public:
    IgnoreSigPipe(){
        ::signal(SIGPIPE, SIG_IGN);
    }
};
}

using namespace jiangbo;

__thread EventLoop *t_thisThreadEventLoop = 0;

IgnoreSigPipe initObj;

const int kPollTimeMs = 10000;

EventLoop::EventLoop()
  : looping_(false), 
    quit_(false),
    callingdoPendingFunctions_(false),
    wakeupfd_(eventfd_create()),
    threadId_(CurrentThread::tid()),
    epoller_(new Epoller(this)),
    timerqueue_(new TimerQueue(this)),
    wakeupchannel_(new Channel(this, wakeupfd_, false))
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << threadId_;
    if(t_thisThreadEventLoop){
      LOG_FATAL << "Another EventLoop "<< t_thisThreadEventLoop
                << " exists in this thread " << threadId_; 
    }else{
        t_thisThreadEventLoop = this;
    }
    wakeupchannel_->setreadCallback(
        boost::bind(&EventLoop::handleRead, this));//绑定wakeupfd的回调
    wakeupchannel_->enableReading();
}

EventLoop::~EventLoop(){
    assert(!looping_);
    ::close(wakeupfd_);//关闭wakeupfd
    t_thisThreadEventLoop = nullptr;
}

void
EventLoop::loop(){
  assertInLoopThread();
  looping_ = true;
  quit_ = false;

  while(!quit_){
    channellist_.clear();
    epollReturnTime_ = epoller_->epoll(kPollTimeMs, &channellist_);
    for(Channel *channel : channellist_){
      channel->handleEvent(epollReturnTime_);
    }
    doPendingFunctions();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

void
EventLoop::runInLoop(const Function &cb){
    //暴露的接口，分配任务给IO线程
    if(isInLoopThread()){
        cb();
    }else{
        queueInLoop(cb);
    }
}

void 
EventLoop::queueInLoop(const Function &cb){
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(cb);
    }
    if(isInLoopThread() || callingdoPendingFunctions_){//非IO线程或者在IO线程doPendingFunctions中立即唤醒任务
        wakeup();
    }
}

TimerId 
EventLoop::runAt(const Timestamp &time, const Function &cb){
    return timerqueue_->addTimer(cb, time, 0.0);
}

TimerId
EventLoop::runAfter(double delay, const Function &cb){
    Timestamp now = Timestamp::now();
    Timestamp time(addTime(now, delay));
    return runAt(time, cb);
}

TimerId
EventLoop::runEvery(double interval, const Function &cb){
    Timestamp when = addTime(Timestamp::now(), interval);
    return timerqueue_->addTimer(cb, when, interval);
}

void
EventLoop::wakeup(){
    uint64_t one = 1;
    ssize_t n = ::write(wakeupfd_, &one, sizeof one);
    if(n != sizeof(one)){
      LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void
EventLoop::quit(){
    quit_ = true;
    if(!isInLoopThread()){
        wakeup();
    }
}

void
EventLoop::updateChannel(Channel *channel){
    assert(channel->ownerEventLoop() == this);
    assertInLoopThread();
    epoller_->updateChannelMap(channel);
}

void
EventLoop::removeChannel(Channel *channel){
    assert(channel->ownerEventLoop() == this);
    assertInLoopThread();
    epoller_->removeChannelMap(channel);

}

void
EventLoop::assertInLoopThread(){
    if(!isInLoopThread()){
        abortNotInLoopThread();
    }
}

void
EventLoop::abortNotInLoopThread(){
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

void 
EventLoop::handleRead(){
  uint64_t one;
  ssize_t n = ::read(wakeupfd_, &one, sizeof one);
  if(n != sizeof one){
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void
EventLoop::doPendingFunctions(){
  callingdoPendingFunctions_ = true;
  std::vector<Function> functions;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    pendingFunctors_.swap(functions);
  }
  for(Function func : functions){
    func();
  }
  callingdoPendingFunctions_ = false;
}

