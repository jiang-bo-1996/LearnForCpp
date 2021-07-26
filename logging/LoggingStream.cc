#include "LoggingStream.h"

#include <algorithm>
#include <boost/static_assert.hpp>
#include <limits>
#include <boost/type_traits/is_arithmetic.hpp>
#include <iostream>
#include <typeinfo>


namespace jiangbo
{
const char digits[] = "9876543210123456789";
const char *zero = digits + 9;
BOOST_STATIC_ASSERT(sizeof(digits) == 20);

const char digitsHex[] = "0123456789abcdef";
BOOST_STATIC_ASSERT(sizeof(digitsHex) == 17);

template<typename T>
size_t convert(char *buf, T v){
    T i = v;
    char *p = buf;
    do{
        int lsd = i % 10;
        i /= 10;
        *p ++ = zero[lsd]; 
    }while(i != 0);

    if(v < 0){
        *p ++ = '-';
    }

    *p = '\0';
    std::reverse(buf, p);
    return static_cast<size_t>(p - buf);
}

size_t convertHex(char *buf, uintptr_t value){
    char *p = buf;
    uintptr_t i = value;
    do{
        int lsd = i % 16;
        i /= 16;
        *p ++ = digitsHex[lsd]; 
    }while (i != 0);

    *p = '\0';
    std::reverse(buf, p);
    return static_cast<size_t>(p - buf);
}

template<typename T>
void LoggingStream::formatInteger(T v){
    if(buffer_.avail() >= kMaxNumericSize){
        size_t len = convert(buffer_.current(), v);
        buffer_.add(len);
    }
}

LoggingStream &LoggingStream::operator<<(short v)
{
    *this<<static_cast<int>(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(unsigned short v)
{
    *this<<static_cast<unsigned int>(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(int v){
    formatInteger(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(unsigned int v){
    formatInteger(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(long v){
    formatInteger(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(unsigned long v){
    formatInteger(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(long long v)
{
    formatInteger(v);
    return *this;
}

LoggingStream &LoggingStream::operator<<(const void *p)
{
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if(buffer_.avail() >= kMaxNumericSize){
        char *buf = buffer_.current();
        buf[0] = '0';
        buf[1] = 'x';
        int len = convertHex(buf + 2, v);
        buffer_.add(len + 2);
    }
    return *this;
}

LoggingStream &LoggingStream::operator<<(double v){
    if(buffer_.avail() >= kMaxNumericSize)
    {
        char *buf = buffer_.current();
        int len = snprintf(buf, kMaxNumericSize, "%.12g", v);
        buffer_.add(len);
    }
    return *this;
}

void LoggingStream::staticCheck()
{
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10);
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10);
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10);
  BOOST_STATIC_ASSERT(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10);
}

template<typename T>
Fmt::Fmt(const char *fmt, T val)
{
    BOOST_STATIC_ASSERT(boost::is_arithmetic<T>::value == true);

    length_ = snprintf(buf_, sizeof buf_, fmt, val);
    assert(static_cast<size_t>(length_) < sizeof buf_);
};
}

using namespace jiangbo;

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);