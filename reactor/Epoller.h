#pragma once

#include "../timestamp/TimeStamp.h"
#include "../thread/Thread.h"
#include "EventLoop.h"

#include <boost/noncopyable.hpp>
#include <vector>
#include <array>
#include <map>
#include <sys/epoll.h>
namespace jiangbo{

class Epoller : boost::noncopyable{
public:
    typedef std::vector<Channel *> ChannelList;
    Epoller(EventLoop *loop);
    ~Epoller();

    Timestamp epoll(int timeoutMs, ChannelList *channelList);

    void assertInLoopThread(){
        loop_->assertInLoopThread();
    }

    void updateChannelMap(Channel *);

    void removeChannelMap(Channel *);
private:
  static const int kInitEventListSize = 16;
  
  void fillActiveChannels(int fdNum, ChannelList *channelList);
  void epollAdd(Channel *channel);
  void epollMod(Channel *channel);
  void epollDel(Channel *channel);
    
  typedef std::map<int, Channel *> ChannelMap;
  typedef std::vector<struct epoll_event> EventList;

  const int epfd_;
  EventLoop *loop_;
  ChannelMap channels_;
  EventList events_;
};
}