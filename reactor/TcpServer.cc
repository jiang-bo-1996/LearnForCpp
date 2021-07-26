#include "TcpServer.h"

#include "EventLoop.h"
#include "InetAddress.h"
#include "Acceptor.h"
#include "TcpConnection.h"
#include "Socketops.h"
#include "EventLoopThreadPool.h"
#include "../logging/Logging.h"

#include <boost/bind.hpp>
#include <boost/get_pointer.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

using namespace jiangbo;

TcpServer::TcpServer(EventLoop *loop, 
                     const InetAddress &listenaddr)
  : loop_(loop),
    name_(listenaddr.toHostPort()),
    started_(false),
    nextConnfd_(1),
    acceptor_(new Acceptor(loop_, listenaddr)),
    threadPool_(new EventLoopThreadPool(loop))
{
    acceptor_->setNewConnectionCallBack(
        boost::bind(&TcpServer::newConnection, this, _1, _2));
}

TcpServer::~TcpServer(){
    
}

void TcpServer::setThreadNum(int numThreads)
{
  assert(0 <= numThreads);
  threadPool_->setThreadNum(numThreads);
}

void
TcpServer::start(){
    if(!started_){
        started_ = true;
        threadPool_->start();
    }

    if(!acceptor_->listenning()){
        loop_->runInLoop(boost::bind(&Acceptor::listen, get_pointer(acceptor_)));    
    }
}

void
TcpServer::newConnection(int sockfd, const InetAddress &peeraddr){
    loop_->assertInLoopThread();
    char buf[32];
    snprintf(buf, sizeof buf, "#%d", nextConnfd_);
    ++ nextConnfd_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_
           << "] - new connection [" << connName
           << "] from " << peeraddr.toHostPort();
    InetAddress localaddr(socketops::getLocalAddr(sockfd));
    EventLoop *ioLoop = threadPool_->getNextLoop();
    TcpConnectionPtr tcpConnectionptr = 
                     boost::make_shared<TcpConnection>(ioLoop, connName, sockfd, localaddr, peeraddr);
    connections_[connName] = tcpConnectionptr;
    tcpConnectionptr->setConnectionCallBack(connectionCallback_);
    tcpConnectionptr->setMessageConnectionCallBack(messageCallback_);
    tcpConnectionptr->setCloseCallBack(
        boost::bind(&TcpServer::removeConnection, this, _1));
    tcpConnectionptr->connectEstablished();
}

void
TcpServer::removeConnection(const TcpConnectionPtr &conn){
   loop_->runInLoop(boost::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void
TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn){
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
    ConnectionMap::iterator iter = connections_.find(conn->name());
    //LOG_DEBUG << "Is connection in map? " << (iter == connections_.end() ? "No" : "Yes"); 
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        boost::bind(&TcpConnection::connectDestoryed, conn));
}
