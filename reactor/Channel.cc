#include "Channel.h"
#include "Epoller.h"
#include "EventLoop.h"
#include "../logging/Logging.h"

#include <sys/epoll.h>

using namespace jiangbo;


const uint32_t Channel::kNoneEvent = 0;
const uint32_t Channel::kReadEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP;
const uint32_t Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd, bool enable_et)
  : loop_(loop), 
    fd_(fd), 
    enableet_(enable_et),
    registeredInEpoller_(false), 
    eventHandling_(false),
    events_(0), 
    revents_(0)
  { }

Channel::~Channel(){
    assert(!eventHandling_);
}

void 
Channel::enableReading(){ 
    events_ |= kReadEvent;
    if(enableet_){
        events_ |= EPOLLET;
    } 
    update(); 
}

void 
Channel::enableWriting(){
    events_ |= kWriteEvent;
    update();
}

bool 
Channel::isWriting() const{
    return events_ & kWriteEvent;
}

void 
Channel::disableWriting(){
    events_ &= ~kWriteEvent;
    update();
}

void 
Channel::handleEvent(Timestamp receiveTime){
    eventHandling_ = true;
    if((revents_ & EPOLLRDHUP) && !(revents_ & EPOLLIN)){
        LOG_WARN << "Channel::handle_event() EPOLLRDHUP";
        if(closeCallback_){
            closeCallback_();
        }
    }
    if(revents_ & EPOLLERR){
        if(errorCallback_){
            errorCallback_();
        }
    }
    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        if(readCallback_){
            readCallback_(receiveTime);
        }
    }
    if(revents_ & EPOLLOUT){
        if(writeCallback_){
            writeCallback_();
        }
    }
    eventHandling_ = false;
}

void 
Channel::update(){
    loop_->updateChannel(this);
}