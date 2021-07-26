#pragma once
#include "Socket.h"
#include "Channel.h"

#include <boost/noncopyable.hpp>
#include <boost/function.hpp>
namespace jiangbo{
class EventLoop;
class Acceptor : boost::noncopyable
{
public:
    typedef boost::function<void (int sockfd, 
                                  const InetAddress&)> NewConnectionCallBack;
    
    Acceptor(EventLoop *loop, const InetAddress &litenAddr);

    void setNewConnectionCallBack(const NewConnectionCallBack &cb)
    { newConnectionCallback_ = cb;}

    bool listenning() const { return listenning_;}
    void listen();
private:
    void handleRead();

    EventLoop *loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallBack newConnectionCallback_;
    bool listenning_;
};
}