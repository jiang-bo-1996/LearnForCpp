#pragma once

#include "../timestamp/TimeStamp.h"
#include "Channel.h"

#include <boost/noncopyable.hpp>
#include <utility>
#include <set>
#include <vector>

namespace jiangbo{
class EventLoop;
class Timer;
class TimerId;
class TimerQueue : boost::noncopyable{
public:
  typedef boost::function<void ()> CallBack;
  TimerQueue(EventLoop *loop);
  ~TimerQueue();

  TimerId addTimer(const CallBack &cb, Timestamp when, double interval);

private:
  typedef std::pair<Timestamp, Timer *> Entry;
  typedef std::set<Entry> TimerList;

  void addTimerInLoop(Timer *timer);
  void handleRead();
  bool insert(Timer *timer);
  void addTimeInLoop(Timer *timer);
  void reset(const std::vector<Entry>& expired, Timestamp now);
  std::vector<Entry> getexpiration(Timestamp now);

  const int timerfd_;
  EventLoop *ownerloop_;
  Channel timerfdchannel_;
  TimerList timerlist_;
};
}