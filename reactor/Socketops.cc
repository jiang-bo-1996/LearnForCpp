#include "Socketops.h"
#include "../logging/Logging.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <boost/implicit_cast.hpp>

namespace jiangbo{
namespace socketops{
typedef struct sockaddr SA;
const SA* 
sockaddr_cast(const sockaddr_in * addr){
    return static_cast<const SA *>(boost::implicit_cast<const void *>(addr));
}

SA* sockaddr_cast(sockaddr_in *addr){
    return static_cast<SA*>(boost::implicit_cast<void *>(addr));
}

void 
setNonblockingAndCloseOnExec(int fd){
    int flags = ::fcntl(fd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(fd, F_SETFL, flags);
    if(ret < 0){
        //输出日志
    }
    flags = ::fcntl(fd, F_GETFD);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(fd, F_SETFD, flags);
}
}
}
using namespace jiangbo;
using namespace socketops;

int 
socketops::createNonblockingOrDie(){
#if VALGRING
    int sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
      LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    }
    
    setNonblockingAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if(sockfd < 0){
      LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    } 
#endif
    return sockfd;     
}

int 
socketops::connect(int sockfd, const struct sockaddr_in &addr){
  return ::connect(sockfd, sockaddr_cast(&addr),sizeof addr);
}

void 
socketops::bindOrDie(int sockfd, const struct sockaddr_in *addr){
    int ret = ::bind(sockfd, sockaddr_cast(addr), sizeof(*addr));
    if(ret < 0){
      LOG_SYSFATAL << "sockets::bindOrDie";
    }
}

void 
socketops::listenOrDie(int sockfd){
    int ret = ::listen(sockfd, SOMAXCONN);
    if(ret < 0){
      LOG_SYSFATAL << "sockets::listenOrDie";
    }
}

int socketops::accept(int listenfd, struct sockaddr_in *addr){
    socklen_t addrlen = sizeof(*addr);
#if VALGRING
    int connfd = ::accept(listenfd, sockaddr_cast(*addr), sizeof(addr));
#else
    int connfd = ::accept4(listenfd, sockaddr_cast(addr), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if(connfd < 0){
        int savedErrno = errno;
        //  
        switch (savedErrno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:
            case EPERM:
            case EMFILE:
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
            break;
            default:
                break;
        }
    }
    return connfd;
}

void 
socketops::closefd(int connfd){
    if(::close(connfd) < 0){
      LOG_SYSERR << "sockets::close";
    }
}

void 
socketops::shutdownWrite(int sockfd){
    if(::shutdown(sockfd, SHUT_WR) < 0){
      LOG_SYSERR << "sockets::shutdownWrite";
    }
}

void 
socketops::toHostPort(char *buf, size_t size,const struct sockaddr_in *addr){
    char host[INET_ADDRSTRLEN] = "INVALID";
    ::inet_ntop(AF_INET, &addr->sin_addr, host, sizeof host);
    uint16_t port = networkToHost16(addr->sin_port);
    snprintf(buf, size, "%s:%u", host, port);
}

void 
socketops::fromHostToPort(const char *ip, uint16_t port, struct sockaddr_in *addr){
  addr->sin_family = AF_INET;
  addr->sin_port = hostToNetwork16(port);
  if(::inet_pton(AF_INET, ip, &addr->sin_addr) < 0){
    LOG_SYSERR << "sockets::fromHostPort";
  }
}

struct sockaddr_in
socketops::getLocalAddr(int sockfd){
  struct sockaddr_in localAddr;
  socklen_t addrlen = sizeof(localAddr);
  ::bzero(&localAddr, sizeof localAddr);
  if(::getsockname(sockfd, sockaddr_cast(&localAddr), &addrlen) < 0){
    LOG_SYSERR << "sockets::getLocalAddr";
  }
  return localAddr;
}

int
socketops::getSocketError(int sockfd){
    int optval;
    socklen_t optlen = sizeof optval;

    if(::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0){
        return errno;
    }else{
        return optval;
    }
}

struct sockaddr_in socketops::getPeerAddr(int sockfd)
{
  struct sockaddr_in peeraddr;
  bzero(&peeraddr, sizeof peeraddr);
  socklen_t addrlen = sizeof(peeraddr);
  if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
  {
    LOG_SYSERR << "sockets::getPeerAddr";
  }
  return peeraddr;
}

bool 
socketops::isSelfConnect(int sockfd)
{
  struct sockaddr_in localaddr = getLocalAddr(sockfd);
  struct sockaddr_in peeraddr = getPeerAddr(sockfd);
  return localaddr.sin_port == peeraddr.sin_port
      && localaddr.sin_addr.s_addr == peeraddr.sin_addr.s_addr;
}
