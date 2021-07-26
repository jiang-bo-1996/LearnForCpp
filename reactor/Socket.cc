#include "Socket.h"
#include "InetAddress.h"
#include "Socketops.h"

#include <string.h>
#include <netinet/tcp.h>
using namespace jiangbo;

Socket::~Socket(){
    socketops::closefd(sockfd_);
}

void
Socket::bindAddress(const InetAddress &localaddr){
    socketops::bindOrDie(sockfd_, &(localaddr.getAddrInet()));
}

void
Socket::listen(){
    socketops::listenOrDie(sockfd_);
}

int
Socket::accept(InetAddress *peeraddr){
    struct sockaddr_in addr;
    ::bzero(&addr, sizeof(addr));
    int connfd = socketops::accept(sockfd_, &addr);
    peeraddr->setAddrInet(addr);
    return connfd;
}

void
Socket::setReuseAddr(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, 
                &optval, sizeof optval);
}

void
Socket::shutdownWrite(){
    socketops::shutdownWrite(sockfd_);
}

void Socket::setTcpNoDelay(bool on){
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
                 &optval, sizeof optval);
                 
}
