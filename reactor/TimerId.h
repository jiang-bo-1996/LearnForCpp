#pragma once

#include "copyable.h"

namespace jiangbo{
class Timer;
class TimerId :public copyable
{
public:
  TimerId(Timer *timer = nullptr, int64_t seq = 0)
    : timer_(timer),
      sequence_(seq)
    { }
private:
  Timer *timer_;
  int64_t sequence_;
};
}