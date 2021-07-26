#pragma once
#include "../thread/Thread.h"
#include "../timestamp/TimeStamp.h"

#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
#include <vector>
#include <mutex>


namespace jiangbo{
class Epoller;
class Channel;
class TimerQueue;
class Timer;
class TimerId;
class EventLoop : boost::noncopyable{
public:
    typedef std::vector<Channel *> ChannelsList;
    typedef boost::function<void ()> Function;
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool isInLoopThread(){
        return threadId_ == CurrentThread::tid();
    }

    void assertInLoopThread();

    void runInLoop(const Function &cb);
    
    void queueInLoop(const Function &cb);

    TimerId runAt(const Timestamp &time, const Function &cb);

    TimerId runAfter(double delay, const Function &cb);

    TimerId runEvery(double interval, const Function &cb);
    
    void wakeup();
private:
    void abortNotInLoopThread();
    void doPendingFunctions();
    void handleRead();
    
    bool looping_;
    bool quit_;
    bool callingdoPendingFunctions_;
    int wakeupfd_;
    const pid_t threadId_;
    Timestamp epollReturnTime_;
    boost::scoped_ptr<Epoller> epoller_;
    boost::scoped_ptr<TimerQueue> timerqueue_;
    boost::scoped_ptr<Channel> wakeupchannel_;
    ChannelsList channellist_;
    std::mutex mutex_;
    std::vector<Function> pendingFunctors_;
};
}