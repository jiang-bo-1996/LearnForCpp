#pragma once

#include "../timestamp/TimeStamp.h"
#include "CallBack.h"
#include <boost/noncopyable.hpp>
#include <boost/function.hpp>

namespace jiangbo{
class EventLoop;
class Channel : boost::noncopyable{
public:
    typedef boost::function<void ()> EventCallBack;
    typedef boost::function<void (Timestamp)> ReadEventCallBack;

    Channel(EventLoop *, int, bool = true);
    ~Channel();

    void enableReading();
    void enableWriting();
    void disableWriting();
    bool isWriting() const;


    uint32_t events() const { return events_; } 
    const int fd() const { return fd_;}
    void setregisteredInEpoller(bool newValue){
        registeredInEpoller_ = newValue;
    }
    bool registeredInEpoller() const{ return registeredInEpoller_;}


    void setrevents(uint32_t event){ revents_ = event;}
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    void setreadCallback(const ReadEventCallBack &cb){ readCallback_ = cb; }
    void setwriteCallback(const EventCallBack &cb){ writeCallback_ = cb; }
    void seterrorCallback(const EventCallBack &cb){ errorCallback_ = cb; }
    void setcloseCallback(const EventCallBack &cb){ closeCallback_ = cb; }
    void disableAll() { events_ = kNoneEvent; update(); }
    
    EventLoop* ownerEventLoop() const{ return loop_; }

    void handleEvent(Timestamp receiveTime);
private:
    void update();
    static const uint32_t kReadEvent;
    static const uint32_t kWriteEvent;
    static const uint32_t kNoneEvent;

    EventLoop *loop_;
    const int fd_;
    bool enableet_;
    bool registeredInEpoller_;
    bool eventHandling_;
    uint32_t events_;
    uint32_t revents_;
    ReadEventCallBack readCallback_;
    EventCallBack writeCallback_;
    EventCallBack errorCallback_;
    EventCallBack closeCallback_;
};
}
