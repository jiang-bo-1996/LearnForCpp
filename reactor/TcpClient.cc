#include "TcpClient.h"

#include "Connector.h"
#include "EventLoop.h"
#include "Socketops.h"
#include "CallBack.h"

#include "../logging/Logging.h"

#include <boost/bind.hpp>

#include <stdio.h> 

using namespace jiangbo;

namespace jiangbo
{
namespace detail
{

void removeConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
  loop->queueInLoop(boost::bind(&TcpConnection::connectDestoryed, conn));
}

void removeConnector(const ConnectorPtr& connector)
{
  //connector->
}

}
}

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& serverAddr)
  : loop_(loop),
    connector_(new Connector(loop, serverAddr)),
    retry_(false),
    connect_(true),
    nextConnId_(1)
{
  connector_->setNewConnectionCallBack(
      boost::bind(&TcpClient::newConnection, this, _1));
  LOG_INFO << "TcpClient::TcpClient[" << this
           << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()
{
  LOG_INFO << "TcpClient::~TcpClient[" << this
           << "] - connector " << get_pointer(connector_);
  TcpConnectionPtr conn;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    conn = connection_;
  }
  if (conn)
  {
    CloseCallBack cb = boost::bind(&detail::removeConnection, loop_, _1);
    loop_->runInLoop(
        boost::bind(&TcpConnection::setCloseCallBack, conn, cb));
  }
  else
  {
    connector_->stop();
    loop_->runAfter(1, boost::bind(&detail::removeConnector, connector_));
  }
}

void TcpClient::connect()
{
  LOG_INFO << "TcpClient::connect[" << this << "] - connecting to "
           << connector_->serverAddress().toHostPort();
  connect_ = true;
  connector_->start();
}

void TcpClient::disconnect()
{
  connect_ = false;

  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connection_)
    {
      connection_->shutdown();
    }
  }
}

void TcpClient::stop()
{
  connect_ = false;
  connector_->stop();
}

void TcpClient::newConnection(int sockfd)
{
  loop_->assertInLoopThread();
  InetAddress peerAddr(socketops::getPeerAddr(sockfd));
  char buf[32];
  snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toHostPort().c_str(), nextConnId_);
  ++ nextConnId_;
  std::string connName = buf;

  InetAddress localAddr(socketops::getLocalAddr(sockfd));

  TcpConnectionPtr conn(new TcpConnection(loop_,
                                          connName,
                                          sockfd,
                                          localAddr,
                                          peerAddr));

  conn->setConnectionCallBack(connectionCallback_);
  conn->setMessageConnectionCallBack(messageCallback_);
  conn->setWriteCompeleteCallback(writeCompleteCallback_);
  conn->setCloseCallBack(
      boost::bind(&TcpClient::removeConnection, this, _1));
  {
    std::lock_guard<std::mutex> lock(mutex_);
    connection_ = conn;
  }
  conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& conn)
{
  loop_->assertInLoopThread();
  assert(loop_ == conn->getLoop());

  {
    std::lock_guard<std::mutex> lock(mutex_);
    assert(connection_ == conn);
    connection_.reset();
  }

  loop_->queueInLoop(boost::bind(&TcpConnection::connectDestoryed, conn));
  if (retry_ && connect_)
  {
    LOG_INFO << "TcpClient::connect[" << this << "] - Reconnecting to "
             << connector_->serverAddress().toHostPort();
    connector_->restart();
  }
}
