#include "Epoller.h"
#include "Channel.h"
#include "../logging/Logging.h"

#include <sys/epoll.h>
#include <assert.h>
#include <iostream>
#include <boost/implicit_cast.hpp>

using namespace jiangbo;


Epoller::Epoller(EventLoop *loop)
  : epfd_(epoll_create(5)), 
    loop_(loop),
    events_(kInitEventListSize)
{ 
  if (epfd_ < 0)
  {
    LOG_SYSFATAL << "EPoller::EPoller";
  }
}

Epoller::~Epoller(){
}

Timestamp 
Epoller::epoll(int timeoutMs, ChannelList *channelList){
  assertInLoopThread();
  int fdNum_  = ::epoll_wait(epfd_, &*events_.begin(), static_cast<int>(events_.size()), timeoutMs);
  Timestamp now = Timestamp::now();
  if(fdNum_ > 0){
    LOG_TRACE << fdNum_ << " events happended";
    fillActiveChannels(fdNum_, channelList);
    if(boost::implicit_cast<size_t>(fdNum_) == events_.size()){
      events_.resize(events_.size() * 2);
    }
  }else if (fdNum_ == 0){
    LOG_TRACE << " nothing happended";
  }else{
    LOG_SYSERR << "EPoller::epoll()";
  }
  return now;
}



void
Epoller::fillActiveChannels(int fdNum, ChannelList *channelList){
  assert(boost::implicit_cast<size_t>(fdNum) <= events_.size());
  for(int i = 0; i < fdNum; ++ i){
        ChannelMap::iterator ch = channels_.find(events_[i].data.fd);
        assert(ch != channels_.end());
        Channel *channel = ch->second;
        channel->setrevents(events_[i].events);
        channelList->push_back(channel);
    }
}

void
Epoller::epollAdd(Channel *channel){
    if(channel->events() > 0){
      assert(channels_.find(channel->fd()) == channels_.end());
      struct epoll_event event;
      event.events = channel->events();
      event.data.fd = channel->fd();
      int ret = ::epoll_ctl(epfd_, EPOLL_CTL_ADD, channel->fd(), &event);
      if(ret == 0){
        channels_.insert({channel->fd(), channel});
        channel->setregisteredInEpoller(true);
      }else{
        LOG_SYSERR << "epoll_ctl op = EPOLL_CTL_ADD" << channel->fd();
      }
    }
}

void
Epoller::epollMod(Channel *channel){
    assert(channels_[channel->fd()] == channel);
    struct epoll_event event;
    ::bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.fd = channel->fd();
    int ret = ::epoll_ctl(epfd_, EPOLL_CTL_MOD, channel->fd(), &event);
    if(ret < 0){
      LOG_SYSERR << "epoll_ctl op = EPOLL_CTL_MOD" << channel->fd();
    }
}

void 
Epoller::epollDel(Channel *channel){
  assert(channels_[channel->fd()] == channel);
  assert(channel->isNoneEvent());
  assert(channel->registeredInEpoller());
  struct epoll_event event;
  event.events = channel->events();
  event.data.fd = channel->fd();
  int ret = ::epoll_ctl(epfd_, EPOLL_CTL_DEL, channel->fd(), &event);
  if(ret == 0){
    channels_.erase(channel->fd());
    channel->setregisteredInEpoller(false);
  }else{
    LOG_SYSERR << "epoll_ctl op = EPOLL_CTL_DEL" << channel->fd();
  }
}

void
Epoller::updateChannelMap(Channel *channel){
  assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  if(!channel->registeredInEpoller()){
    epollAdd(channel);
  }else{
    epollMod(channel);
  }
}

void
Epoller::removeChannelMap(Channel *channel){
  assertInLoopThread();
  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
  epollDel(channel);
}