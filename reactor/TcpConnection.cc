#include "TcpConnection.h"
#include "Socket.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socketops.h"
#include "../logging/Logging.h"

#include <boost/bind.hpp>
#include <boost/get_pointer.hpp>
#include <boost/implicit_cast.hpp>

using namespace jiangbo;
using namespace detail;

TcpConnection::TcpConnection(EventLoop *loop,
                const std::string &name,
                int sockfd,
                const InetAddress &localAddr,
                const InetAddress &peerAddr)
  : loop_(loop),
    state_(kConnecting),
    name_(name),
    socket_(new Socket(sockfd)),
    channel_(new Channel(loop_, socket_->fd(), false)),
    localAddr_(localAddr),
    peerAddr_(peerAddr_)
{
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at " << this
              << " fd="<< sockfd;
    channel_->setreadCallback(
            boost::bind(&TcpConnection::handleRead, this, _1));
    channel_->setcloseCallback(
            boost::bind(&TcpConnection::handleClose, this));
    channel_->setwriteCallback(
            boost::bind(&TcpConnection::handleWrite, this));
    channel_->seterrorCallback(
            boost::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection(){
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
              << " fd=" << channel_->fd();
}

void
TcpConnection::send(const std::string &message){
    if(state_ == kConnected){
        if(loop_->isInLoopThread()){
            sendInLoop(message);
        }else{
            loop_->runInLoop(
                boost::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void
TcpConnection::sendInLoop(const std::string &message){
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    if(!channel_->isWriting() && outputBuffer_.readableBytes()){
        nwrote = ::write(channel_->fd(),
                         message.data(),
                         message.size());
        if(nwrote >= 0){
            if(boost::implicit_cast<size_t>(nwrote) < message.size()){
                LOG_TRACE << "I am going to write more data";
            }else if(writeCompleteCallback_){
                loop_->queueInLoop(boost::bind(writeCompleteCallback_, shared_from_this()));
            }
        }else{
            nwrote = 0;
            if(errno != EWOULDBLOCK){
                LOG_SYSERR << "TcpConnection::sendInLoop";
            }
        }
    }

    assert(nwrote >= 0);
    if(boost::implicit_cast<size_t>(nwrote) < message.size()){
        outputBuffer_.append(message.data() + nwrote, message.size() - nwrote);
        if(!channel_->isWriting()){
            channel_->enableWriting();
        }
    }
}

void 
TcpConnection::shutdown(){
    if(state_ == kConnected){
        setState(kDisconnecting);
        loop_->runInLoop(
            boost::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void
TcpConnection::shutdownInLoop(){
    loop_->assertInLoopThread();
    if(!channel_->isWriting()){
        socket_->shutdownWrite();
    }
}

void
TcpConnection::connectEstablished(){
    loop_->assertInLoopThread();
    assert(state_ == kConnecting);
    channel_->enableReading();
    setState(kConnected);

    connectionCallback_(shared_from_this());
}

void
TcpConnection::handleRead(Timestamp receiveTime){
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if(n > 0){
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    }else if(n == 0){
        handleClose();
    }else{
        errno = savedErrno;
        LOG_SYSERR << "TcpConnection::handleRead";
        handleError();
    }
}

void
TcpConnection::handleWrite(){
    loop_->assertInLoopThread();
    if(channel_->isWriting()){
        ssize_t n = ::write(channel_->fd(),
                            outputBuffer_.peek(),
                            outputBuffer_.readableBytes());
        if(n > 0){
            outputBuffer_.retrieve(n);
            if(outputBuffer_.readableBytes() == 0){
                channel_->disableWriting();
                if(writeCompleteCallback_){
                    loop_->queueInLoop(
                        boost::bind(writeCompleteCallback_, shared_from_this()));
                }
                if(state_ == kDisconnecting){
                    shutdownInLoop();
                }
            }else{
                LOG_TRACE << "I am going to write more data";
            }
        }else{
            LOG_SYSERR << "TcpConnection::handleWrite";
        }
    }else{
        LOG_TRACE << "Connection is down, no more writing";
    }
}

void
TcpConnection::handleError(){
    int err = socketops::getSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::handleError [" << name_
            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

void 
TcpConnection::handleClose(){
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << state_;
    assert(state_ == kConnected  || state_ == kDisconnecting);
    channel_->disableAll();

    closeCallback_(shared_from_this());
}

void
TcpConnection::connectDestoryed(){
    loop_->assertInLoopThread();
    assert(state_ == kConnected || state_ == kDisconnecting);
    setState(kDisConnected);
    channel_->disableAll();
    connectionCallback_(shared_from_this());

    loop_->removeChannel(get_pointer(channel_));
}

void TcpConnection::setTcpNoDelay(bool on){
  socket_->setTcpNoDelay(on);
}