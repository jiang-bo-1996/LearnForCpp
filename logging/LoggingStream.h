#pragma once

#include <boost/noncopyable.hpp>
#include <string.h>
#include <string>
#include <assert.h>
#include <stdio.h>

namespace jiangbo
{
    const int kMinBufferSize = 4000;
    const int kMaxBufferSize = 4000 * 1000;
template<int SIZE>
class FixedBuffer : boost::noncopyable
{
public:
    FixedBuffer()
      : cur_(data_)
      { }
    void append(const char * /*restrict*/ buf, int len)
    {
        if(avail() > len){
            memcpy(cur_, buf, len);
            cur_ += len;
        }
    }
    void add(int len){
        cur_ += len;
    }
    int length() const { return static_cast<int>(cur_ - data_); }//指针相减是ptrdiff_t
    int avail() const { return static_cast<int>(end() - cur_); }
    char *current() const { return cur_; }
    void reset() { cur_ = data_; }
    const char *data() const { return data_; }
    void bzero() { ::bzero(data_, sizeof data_);}
    std::string tostring() const{ return std::string(data_, length()); }
private:
    const char *end() const { return data_ + sizeof data_; }
    char data_[SIZE];
    char *cur_;
};

class T
{
public:
  T(const char* str, int len)
    :str_(str),
     len_(len)
  {
    assert(strlen(str) == len_);
  }

  const char* str_;
  const size_t len_;
};

class LoggingStream : boost::noncopyable
{
public:
    typedef FixedBuffer<kMinBufferSize> Buffer;
    typedef LoggingStream self;

    LoggingStream(){
        buffer_.bzero();
    }
    self &operator<<(bool v){
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }
    self &operator<<(char v){
        buffer_.append(&v, 1);
        return *this;
    }

    self &operator<<(short);
    self &operator<<(unsigned short);
    self &operator<<(int);
    self &operator<<(unsigned int);
    self &operator<<(long);
    self &operator<<(unsigned long);
    self &operator<<(long long);
    self &operator<<(unsigned long long);

    self &operator<<(double);
    self &operator<<(float v){
        *this << static_cast<double>(v);
        return *this;
    }

    self &operator<<(const char* buf){
        buffer_.append(buf, strlen(buf));
        return *this;
    }

    self &operator<<(const std::string& str){
        buffer_.append(str.c_str(), str.length());
        return *this;
    }

    self &operator<<(const T &v)
    {
        buffer_.append(v.str_, v.len_);
        return *this;
    }

    self &operator<<(const void *p); 

    void append(const char *buf, int len){
        buffer_.append(buf, len);
    }
    const Buffer& buffer() const { return buffer_;}
    void resetBuffer(){ buffer_.reset(); }
private:
    void staticCheck();
    template<typename T>
    void formatInteger(T);
    Buffer buffer_;

    static const int kMaxNumericSize = 32;
};


class Fmt // : boost::noncopyable
{
public:
  template<typename T>
  Fmt(const char* fmt, T val);

  const char* data() const { return buf_; }
  int length() const { return length_; }

private:
  char buf_[32];
  int length_;
};

inline LoggingStream& operator<<(LoggingStream& s, const Fmt& fmt)
{
  s.append(fmt.data(), fmt.length());
  return s;
}
}