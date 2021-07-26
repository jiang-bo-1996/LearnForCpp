#include "InetAddress.h"
#include "Socketops.h"

#include <netinet/in.h>
#include <string.h>
#include <boost/static_assert.hpp>

using namespace jiangbo;
static const in_addr_t kInaddrAny = socketops::hostToNetwork32(INADDR_ANY);

BOOST_STATIC_ASSERT(sizeof(InetAddress) == sizeof(struct sockaddr_in));

InetAddress::InetAddress(uint16_t port){
    ::bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = kInaddrAny;
    addr_.sin_port = socketops::hostToNetwork16(port);
}

InetAddress::InetAddress(const std::string &ip, uint16_t port){
    ::bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    socketops::fromHostToPort(ip.c_str(), port, &addr_);
}

std::string InetAddress::toHostPort() const{
    char buf[32];
    ::bzero(buf, sizeof(buf));
    socketops::toHostPort(buf, sizeof(buf), &addr_);
    return buf;
}