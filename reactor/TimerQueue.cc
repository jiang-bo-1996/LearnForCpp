#include "TimerQueue.h"
#include "EventLoop.h"
#include "Timer.h"
#include "TimerId.h"
#include "../logging/Logging.h"
#include "../thread/Atomic.h"

#include <sys/timerfd.h>
#include <boost/bind.hpp>
#include <iostream>

namespace jiangbo{
int createtimerfd(){
    int timerfd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if(timerfd_create < 0){
      LOG_SYSFATAL << "Failed in timerfd_create";
    }
    return timerfd_;
}

struct timespec howMachTimeFromNow(Timestamp when){
    timespec ts;
    int64_t now = Timestamp::now().microSecondsSinceEpoch();
    int64_t microSecond = when.microSecondsSinceEpoch() - now;
    if(microSecond < 100){
        microSecond = 100;
    }
    ts.tv_sec = static_cast<time_t>(microSecond / Timestamp::kMicroSecondPerSecond);
    ts.tv_nsec = static_cast<long>((microSecond % Timestamp::kMicroSecondPerSecond)* 1000);
    return ts;
}

void readTimerfd(int timerfd, Timestamp now){
  uint64_t howmany;
  ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
  LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
  if(n != sizeof(howmany)){
    LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
  }
}

void resetTimerfd(int timerfd, Timestamp expiration){
  struct itimerspec oldValue;
  struct itimerspec newValue;
  bzero(&newValue, sizeof newValue);
  bzero(&oldValue, sizeof oldValue);
  newValue.it_value = howMachTimeFromNow(expiration);
  int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
  if(ret){
    LOG_SYSERR << "timerfd_settime()";
  }
}
}

using namespace jiangbo;

AtomicInt64 Timer::s_numCreated_;

TimerQueue::TimerQueue(EventLoop *loop)
  : timerfd_(createtimerfd()),
    ownerloop_(loop),
    timerfdchannel_(loop, timerfd_, false)
{
    timerfdchannel_.setreadCallback(boost::bind(&TimerQueue::handleRead, this));
    timerfdchannel_.enableReading();
}

TimerQueue::~TimerQueue(){
    ::close(timerfd_);
    for(TimerList::iterator iter = timerlist_.begin(); iter != timerlist_.end(); ++ iter){
        delete iter->second;
    }
}

TimerId
TimerQueue::addTimer(const CallBack &cb, Timestamp when, double interval){
    Timer *timer = new Timer(cb, when, interval);
    ownerloop_->runInLoop(boost::bind(&TimerQueue::addTimerInLoop, this, timer));
    return TimerId(timer, timer->sequence());
}

void TimerQueue::addTimerInLoop(Timer *timer){
    ownerloop_->assertInLoopThread();
    bool earlyestChange = insert(timer);

    if(earlyestChange){
        resetTimerfd(timerfd_, timer->expiration());
    }
}

void
TimerQueue::handleRead(){
    ownerloop_->assertInLoopThread();
    Timestamp now = Timestamp::now();
    readTimerfd(timerfd_, now);
    std::vector<Entry> expirated = getexpiration(now);

    for(std::vector<Entry>::iterator iter = expirated.begin();
        iter != expirated.end(); ++ iter)
    {
        iter->second->run();
    }

    reset(expirated, now);
}

void
TimerQueue::reset(const std::vector<Entry> &expirated, Timestamp now){
    
    Timestamp nextExpire;
    for(std::vector<Entry>::const_iterator iter = expirated.begin(); 
        iter != expirated.end(); ++ iter)
    {
        if(iter->second->repeat()){
            iter->second->restart(now);
            insert(iter->second);
        }else{
            delete iter->second;
        }
    }
    if(!timerlist_.empty()){
        nextExpire = timerlist_.begin()->first;
    }
    if(nextExpire.valid()){
        resetTimerfd(timerfd_, nextExpire);
    }
}

std::vector<TimerQueue::Entry>
TimerQueue::getexpiration(Timestamp now){
    std::vector<Entry> expirated;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer *>(UINTPTR_MAX));
    TimerList::iterator iter = timerlist_.lower_bound(sentry);
    assert(iter == timerlist_.end() || now < iter->first);

    std::copy(timerlist_.begin(), iter, std::back_inserter(expirated));
    timerlist_.erase(timerlist_.begin(), iter);

    return expirated;
}

bool
TimerQueue::insert(Timer *timer){
    bool earlyestChange = false;
    Timestamp when = timer->expiration();
    TimerList::iterator first = timerlist_.begin();
    
    if(first == timerlist_.end() || when < first->first){
        earlyestChange = true;
    }
    std::pair<TimerList::iterator, bool> ret = 
            timerlist_.insert({when, timer});
    assert(ret.second);
    return earlyestChange;
}