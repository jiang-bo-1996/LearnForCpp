#pragma once
#include "copyable.h"

#include <string>
#include <netinet/in.h>
namespace jiangbo{
class InetAddress{
public:
    explicit InetAddress(uint16_t port);
    InetAddress(const std::string &ip, uint16_t port);
    InetAddress(const sockaddr_in &addr)
      : addr_(addr)
    { }
    std::string toHostPort() const;

    const struct sockaddr_in &getAddrInet()const { return addr_;}
    void setAddrInet(const sockaddr_in &addr){ addr_ = addr;}
private:
    struct sockaddr_in addr_;
};
}