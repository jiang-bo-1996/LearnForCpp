#include "Connector.h"

#include "Channel.h"
#include "EventLoop.h"
#include "Socketops.h"

#include "../logging/Logging.h"

#include <boost/bind.hpp>

#include <errno.h>

using namespace jiangbo;

const int Connector::kMaxRetryDelayMs;

Connector::Connector(EventLoop *loop, const InetAddress &serverAddr)
  : loop_(loop),
    serverAddr_(serverAddr),
    connect_(false),
    state_(kDisconnected),
    retryDelayMs_(kInitRetryDalayMs)
{ 
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector(){
  LOG_DEBUG << "dtor[" << this << "]";
  assert(!channel_);
}

void Connector::start(){
  connect_ = true;
  loop_->runInLoop(boost::bind(&Connector::startInLoop, this));
}

void Connector::startInLoop(){
  loop_->assertInLoopThread();
  assert(state_ == kDisconnected);
  if(connect_){
    connect();
  }else{
    LOG_DEBUG << "do not connect";
  }
}

void Connector::connect(){
  int sockfd = socketops::createNonblockingOrDie();

  int ret = socketops::connect(sockfd, serverAddr_.getAddrInet());
  int savedErrno = (ret == 0) ? 0 : errno;
  switch(savedErrno){
    case 0 : 
    case EINPROGRESS :
    case EINTR :
    case EISCONN :
      connecting(sockfd);
      break;
    
    case EAGAIN :
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sockfd);
      break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << savedErrno;
      socketops::closefd(sockfd);
      break;

    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop " << savedErrno;
      socketops::closefd(sockfd);
      // connectErrorCallback_();
      break;
  }
}

void Connector::restart(){
  loop_->assertInLoopThread();
  setState(kDisconnected);
  retryDelayMs_ = kInitRetryDalayMs;
  connect_ = true;
  startInLoop();
}

void Connector::stop(){
  connect_ = false;
}

void Connector::connecting(int sockfd){
  setState(kConnecting);
  assert(!channel_);
  channel_.reset(new Channel(loop_, sockfd));
  channel_->setwriteCallback(
      boost::bind(&Connector::handleWrite, this));
  channel_->seterrorCallback(
      boost::bind(&Connector::handleError, this));
  channel_->enableWriting();
}

int Connector::removeAndResetChannel(){
  channel_->disableAll();
  loop_->removeChannel(boost::get_pointer(channel_));
  int sockfd = channel_->fd();
  loop_->queueInLoop(boost::bind(&Connector::resetChannel, this));
  return sockfd;
}

void Connector::resetChannel(){
  channel_.reset();
}

void Connector::handleWrite()
{
  LOG_TRACE << "Connector::handleWrite " << state_;

  if (state_ == kConnecting)
  {
    int sockfd = removeAndResetChannel();
    int err = socketops::getSocketError(sockfd);
    if (err)
    {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = "
               << err << " " << strerror_tl(err);
      retry(sockfd);
    }
    else if (socketops::isSelfConnect(sockfd))
    {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sockfd);
    }
    else
    {
      setState(kConnected);
      if (connect_)
      {
        newConnectionCallBack_(sockfd);
      }
      else
      {
        socketops::closefd(sockfd);
      }
    }
  }
  else
  {
    assert(state_ == kDisconnected);
  }
}

void Connector::handleError()
{
  LOG_ERROR << "Connector::handleError";
  assert(state_ == kConnecting);

  int sockfd = removeAndResetChannel();
  int err = socketops::getSocketError(sockfd);
  LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
  retry(sockfd);
}

void Connector::retry(int sockfd)
{
  socketops::closefd(sockfd);
  setState(kDisconnected);
  if (connect_)
  {
    LOG_INFO << "Connector::retry - Retry connecting to "
             << serverAddr_.toHostPort() << " in "
             << retryDelayMs_ << " milliseconds. ";
    timerId_ = loop_->runAfter(retryDelayMs_ / 1000.0,  // FIXME: unsafe
                               boost::bind(&Connector::startInLoop, this));
    retryDelayMs_ = std::min(retryDelayMs_ * 2, kMaxRetryDelayMs);
  }
  else
  {
    LOG_DEBUG << "do not connect";
  }
}

