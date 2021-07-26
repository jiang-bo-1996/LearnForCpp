#pragma once

#include "../timestamp/TimeStamp.h"
#include "../thread/Atomic.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace jiangbo
{
class Timer : boost::noncopyable{
public:
    typedef boost::function<void ()> TimerCallBack;

    Timer(const TimerCallBack &cb, Timestamp when, double interval)
      : callback_(cb),
        expiration_(when),
        interval_(interval),
        repeat_(interval > 0.0),
        sequence_(s_numCreated_.incrementAndGet())
    { }
    
    void restart(Timestamp);
    
    void run() const { callback_(); }

    Timestamp expiration() { return expiration_; }

    bool repeat() const { return repeat_; }

    int64_t sequence() const { return sequence_; }
private:
    const double interval_;
    const bool repeat_;
    Timestamp expiration_;
    const TimerCallBack callback_;
    const int64_t sequence_;

    static AtomicInt64 s_numCreated_;
};
} // namespace jiangbo
