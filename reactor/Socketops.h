#pragma once

#include <netinet/in.h>
namespace jiangbo{
namespace socketops{
inline
uint64_t hostToNetwork64(uint64_t ip){
    return htobe64(ip);
}

inline
uint32_t hostToNetwork32(uint32_t ip){
    return htonl(ip);
}

inline
uint16_t hostToNetwork16(uint16_t ip){
    return htons(ip);
}

inline
uint64_t networkToHost64(uint64_t ip){
    return be64toh(ip);
}

inline
uint32_t networkToHost32(uint32_t ip){
    return ntohl(ip);
}

inline uint32_t networkToHost16(uint16_t ip){
    return ntohs(ip);
}

int createNonblockingOrDie();

int connect(int sockfd, const struct sockaddr_in &addr);

void bindOrDie(int sockfd, const struct sockaddr_in *addr);

void listenOrDie(int sockfd);

int accept(int listenfd, struct sockaddr_in *addr);

void closefd(int connfd);

void shutdownWrite(int sockfd);

void toHostPort(char *buf, size_t size, 
                const struct sockaddr_in *addr);

void fromHostToPort(const char *ip, uint16_t port, 
                    struct sockaddr_in *addr);

struct sockaddr_in getLocalAddr(int sockfd);

struct sockaddr_in getPeerAddr(int sockfd);

int getSocketError(int sockfd);

bool isSelfConnect(int sockfd);
};
}