#pragma once 
#include "LoggingStream.h"
#include "../timestamp/TimeStamp.h"

namespace jiangbo
{
class Logger
{
public:
enum LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    NUM_LOG_LEVELS,
};

Logger(const char *file, int line);
Logger(const char *file, int line, LogLevel level);
Logger(const char *file, int line, LogLevel level, const char *func);
Logger(const char *file, int line, bool toAbort);
~Logger();

LoggingStream & stream() { return impl_.stream_;}

static LogLevel logLevel();
static void setLogLevel(LogLevel level);

typedef void (*OutputFunc)(const char *msg, int len);
typedef void (*FlushFunc)();

static void setOutput(OutputFunc);
static void setFlush(FlushFunc);
private:
class Impl
{
public:
    typedef Logger::LogLevel LogLevel;
    Impl(LogLevel level, int old_errno, const char *file, int line);
    void formatTime();
    void finish();

    Timestamp time_;
    LoggingStream stream_;
    LogLevel level_;
    int line_;
    const char *fullname_;
    const char *basename_;
};
  Impl impl_;
};

#define LOG_TRACE if (jiangbo::Logger::logLevel() <= jiangbo::Logger::TRACE) \
  jiangbo::Logger(__FILE__, __LINE__, jiangbo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (jiangbo::Logger::logLevel() <= jiangbo::Logger::DEBUG) \
  jiangbo::Logger(__FILE__, __LINE__, jiangbo::Logger::DEBUG, __func__).stream()
#define LOG_INFO if (jiangbo::Logger::logLevel() <= jiangbo::Logger::INFO) \
  jiangbo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN jiangbo::Logger(__FILE__, __LINE__, jiangbo::Logger::WARN).stream()
#define LOG_ERROR jiangbo::Logger(__FILE__, __LINE__, jiangbo::Logger::ERROR).stream()
#define LOG_FATAL jiangbo::Logger(__FILE__, __LINE__, jiangbo::Logger::FATAL).stream()
#define LOG_SYSERR jiangbo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL jiangbo::Logger(__FILE__, __LINE__, false).stream()


const char* strerror_tl(int savedErrno);
}

