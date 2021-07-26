#include "LoggingStream.h"

#include <limits>
#include <stdint.h>
#include <stdio.h>

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>

using std::string;
BOOST_AUTO_TEST_CASE(TestLoggingStreamBooleans){
    jiangbo::LoggingStream os;
    const jiangbo::LoggingStream::Buffer &buffer = os.buffer();
    BOOST_CHECK_EQUAL(buffer.tostring(), std::string(""));
    os << true;
    BOOST_CHECK_EQUAL(buffer.tostring(), std::string("1"));
    os << '\n';
    BOOST_CHECK_EQUAL(buffer.tostring(), std::string("1\n"));
    os<<false;
    BOOST_CHECK_EQUAL(buffer.tostring(), std::string("1\n0"));
};

BOOST_AUTO_TEST_CASE(testLogStreamIntegers)
{
  jiangbo::LoggingStream os;
  const jiangbo::LoggingStream::Buffer& buf = os.buffer();
  BOOST_CHECK_EQUAL(buf.tostring(), string(""));
  os << 1;
  BOOST_CHECK_EQUAL(buf.tostring(), string("1"));
  os << 0;
  BOOST_CHECK_EQUAL(buf.tostring(), string("10"));
  os << -1;
  BOOST_CHECK_EQUAL(buf.tostring(), string("10-1"));
  os.resetBuffer();

  os << 0 << " " << 123 << 'x' << 0x64;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0 123x100"));
}


BOOST_AUTO_TEST_CASE(testLogStreamIntegerLimits)
{
    jiangbo::LoggingStream os;
    const jiangbo::LoggingStream::Buffer &buffer_ = os.buffer();
    os << -2147483647;
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("-2147483647"));
    os << (int)-2147483648;
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("-2147483647-2147483648"));
    os << ' ';
    os << 2147483647;
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("-2147483647-2147483648 2147483647"));
    os.resetBuffer();

    os<<std::numeric_limits<int16_t>::min();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("-32768"));
    os.resetBuffer();

    os<<std::numeric_limits<unsigned int16_t>::min();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("0"));
    os.resetBuffer();

    os << std::numeric_limits<unsigned int16_t>::max();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("0"));
    os.resetBuffer();
    
    os << std::numeric_limits<int32_t>::min();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("-2147483648"));
    os.resetBuffer();

    os << std::numeric_limits<int32_t>::max();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("2147483647"));
    os.resetBuffer();

    os << std::numeric_limits<unsigned int>::min();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("0"));
    os.resetBuffer();

    os << std::numeric_limits<unsigned int>::max();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("4294967295"));
    os.resetBuffer();

    os << std::numeric_limits<long>::min();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("-9223372036854775808"));
    os.resetBuffer();

    os << std::numeric_limits<long>::max();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("9223372036854775807"));
    os.resetBuffer();

    os << std::numeric_limits<unsigned long>::min();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("0"));
    os.resetBuffer();

    os << std::numeric_limits<unsigned long>::max();
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("18446744073709551615"));
    os.resetBuffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    os << a;
    os << b;
    os << c;
    BOOST_CHECK_EQUAL(buffer_.tostring(), string("000"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFloats)
{
  jiangbo::LoggingStream os;
  const jiangbo::LoggingStream::Buffer& buf = os.buffer();

  os << 0.0;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0"));
  os.resetBuffer();

  os << 1.0;
  BOOST_CHECK_EQUAL(buf.tostring(), string("1"));
  os.resetBuffer();

  os << 0.1;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.1"));
  os.resetBuffer();

  os << 0.05;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.05"));
  os.resetBuffer();

  os << 0.15;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.15"));
  os.resetBuffer();

  double a = 0.1;
  os << a;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.1"));
  os.resetBuffer();

  double b = 0.05;
  os << b;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.05"));
  os.resetBuffer();

  double c = 0.15;
  os << c;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.15"));
  os.resetBuffer();

  os << a+b;
  BOOST_CHECK_EQUAL(buf.tostring(), string("0.15"));
  os.resetBuffer();

  BOOST_CHECK(a+b != c);

  os << 1.23456789;
  BOOST_CHECK_EQUAL(buf.tostring(), string("1.23456789"));
  os.resetBuffer();

  os << 1.234567;
  BOOST_CHECK_EQUAL(buf.tostring(), string("1.234567"));
  os.resetBuffer();

  os << -123.456;
  BOOST_CHECK_EQUAL(buf.tostring(), string("-123.456"));
  os.resetBuffer();
}


BOOST_AUTO_TEST_CASE(testLogStreamVoid)
{
  jiangbo::LoggingStream os;
  const jiangbo::LoggingStream::Buffer& buf = os.buffer();

  os << static_cast<void*>(0);
  BOOST_CHECK_EQUAL(buf.tostring(), string("0x0"));
  os.resetBuffer();

  os << reinterpret_cast<void*>(8888);
  BOOST_CHECK_EQUAL(buf.tostring(), string("0x22b8"));
  os.resetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamFmts)
{
  jiangbo::LoggingStream os;
  const jiangbo::LoggingStream::Buffer& buf = os.buffer();

  os << jiangbo::Fmt("%4d", 1);
  BOOST_CHECK_EQUAL(buf.tostring(), string("   1"));
  os.resetBuffer();

  os << jiangbo::Fmt("%4.2f", 1.2);
  BOOST_CHECK_EQUAL(buf.tostring(), string("1.20"));
  os.resetBuffer();

  os << jiangbo::Fmt("%4.2f", 1.2) << jiangbo::Fmt("%4d", 43);
  BOOST_CHECK_EQUAL(buf.tostring(), string("1.20  43"));
  os.resetBuffer();
}