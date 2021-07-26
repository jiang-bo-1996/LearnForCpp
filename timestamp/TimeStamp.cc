#include "TimeStamp.h"

#include <sys/time.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS

#include <boost/static_assert.hpp>

using namespace jiangbo;

BOOST_STATIC_ASSERT(sizeof(Timestamp) == sizeof(int64_t));

Timestamp::Timestamp()
  : microSecondsSinceEpoch_(0)
{ }

Timestamp::Timestamp(int64_t microSecondsSinceEpoch)
  : microSecondsSinceEpoch_(microSecondsSinceEpoch)
{ }


std::string Timestamp::toString() const
{
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMicroSecondPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMicroSecondPerSecond;
    snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return buf;
}

std::string Timestamp::toFormattedString() const{
    char buf[64] = {0};
    time_t seconds = static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondPerSecond);
    int microsecconds = static_cast<int>(microSecondsSinceEpoch_ % kMicroSecondPerSecond);
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);

    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%06d",
        tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
        tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec, microsecconds);
    return buf;
}

Timestamp Timestamp::now(){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMicroSecondPerSecond + tv.tv_usec);
}
Timestamp Timestamp::invaild()
{
    return Timestamp();
}




