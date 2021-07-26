#include "Logging.h"
#include "../thread/Thread.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sstream>

namespace jiangbo
{
    __thread char t_errnobuf[512];
    __thread char t_time[32];
    __thread time_t t_lastSecond;
    const char *strerror_tl(int savedErrno)
    {
        return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
    }

    Logger::LogLevel initLogLevel()
    {
        if(::getenv("JIANGBO_LOG_TRACE"))
        {
            return Logger::TRACE;
        }else{
            return Logger::DEBUG;
        }
    }

    Logger::LogLevel g_logLevel = initLogLevel();

    const char *LogLevelname[Logger::NUM_LOG_LEVELS] = 
    {
        "[TRACE] ",
        "[DEBUG] ",
        "[INFO ] ",
        "[WARN ] ",
        "[ERROR] ",
        "[FATAL] ",
    };

    void defaultOutput(const char *msg, int len){
        size_t n = fwrite(msg, 1, len, stdout);
        (void)n;
    }

    void defaultFlush()
    {
        fflush(stdout);
    }

    Logger::OutputFunc g_output = defaultOutput;
    Logger::FlushFunc g_flush = defaultFlush;
}

using namespace jiangbo;

Logger::Impl::Impl(LogLevel level, int SavedErrno, const char *file, int line)
  : time_(Timestamp::now()),
    stream_(),
    level_(level),
    line_(line),
    fullname_(file),
    basename_(NULL)
{
  const char *path_sep_pos = strrchr(fullname_, '/');
  basename_ = (path_sep_pos != NULL ? path_sep_pos + 1 : fullname_);

  formatTime();
  Fmt tid("[%6d] ", jiangbo::CurrentThread::tid());
  assert(tid.length() == 9);
  stream_ << T(tid.data(), 9);
  stream_ << T(LogLevelname[level], 8);

  if(SavedErrno != 0)
  {
    stream_ << strerror_tl(SavedErrno) << " (errno=" << SavedErrno << ") ";
  }
}

void Logger::Impl::formatTime()
{
  int64_t microSecondsSinceEpoch = time_.microSecondsSinceEpoch();
  time_t seconds = static_cast<time_t>(microSecondsSinceEpoch / 1000000);
  int microseconds = static_cast<int>(microSecondsSinceEpoch % 1000000);
  if(seconds != t_lastSecond){
      t_lastSecond = seconds;
      struct tm tm_time;
      ::gmtime_r(&seconds, &tm_time);

      int len = snprintf(t_time, sizeof(t_time), "[%4d%02d%02d] [%02d:%02d:%02d", tm_time.tm_year + 1900,
                         tm_time.tm_mon + 1, tm_time.tm_mday, tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
      assert(len == 20);
    }
    Fmt us(".%06dZ] ", microseconds);
    assert(us.length() == 10);
    T temp(us.data(), 10);
    stream_ << T(t_time, 20) << T(us.data(), 10);
}

void Logger::Impl::finish()
{
    stream_ << "-" << basename_ << ":" << line_ << "\n";
}

Logger::Logger(const char* file, int line)
  : impl_(INFO, 0, file, line)
{
}

Logger::Logger(const char* file, int line, LogLevel level, const char* func)
  : impl_(level, 0, file, line)
{
  impl_.stream_ << func << ' ';
}

Logger::Logger(const char* file, int line, LogLevel level)
  : impl_(level, 0, file, line)
{
}

Logger::Logger(const char* file, int line, bool toAbort)
  : impl_(toAbort?FATAL:ERROR, errno, file, line)
{
}

Logger::~Logger()
{
  impl_.finish();
  const LoggingStream::Buffer& buf(stream().buffer());
  g_output(buf.data(), buf.length());
  if (impl_.level_ == FATAL)
  {
    g_flush();
    abort();
  }
}

Logger::LogLevel Logger::logLevel()
{
  return g_logLevel;
}

void Logger::setLogLevel(Logger::LogLevel level)
{
  g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
  g_output = out;
}

void Logger::setFlush(FlushFunc flush)
{
  g_flush = flush;
}