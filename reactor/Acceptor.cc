#include "EventLoop.h"
#include "Acceptor.h"
#include "Socketops.h"
#include "InetAddress.h"

#include <boost/bind.hpp>

using namespace jiangbo;

Acceptor::Acceptor(EventLoop *loop, const InetAddress &listenAddr)
  : loop_(loop), 
    acceptSocket_(socketops::createNonblockingOrDie()),
    acceptChannel_(loop_, acceptSocket_.fd(), false),
    listenning_(false)
{
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.bindAddress(listenAddr);
    acceptChannel_.setreadCallback(boost::bind(&Acceptor::handleRead, this));
}

void Acceptor::listen(){
    loop_->assertInLoopThread();
    acceptChannel_.enableReading();
    listenning_ = true;
    socketops::listenOrDie(acceptSocket_.fd());
}

void Acceptor::handleRead(){
    loop_->assertInLoopThread();
    InetAddress addr(0);
    int connfd = acceptSocket_.accept(&addr);
    if(connfd >= 0){
        if(newConnectionCallback_){
            newConnectionCallback_(connfd, addr);
        }else{
            socketops::closefd(connfd);
        }
    }
}