#pragma once
#include <stdint.h>
#include <string>
#include <algorithm>

namespace jiangbo
{
class Timestamp
{
public:
Timestamp();

explicit Timestamp(int64_t microSecondsSinceEpoch);

void swap(Timestamp& that)
{
  std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
}

std::string toString() const;
std::string toFormattedString() const;
bool valid() const { return microSecondsSinceEpoch_ > 0; }
int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_;}

static Timestamp now();
static Timestamp invaild();
static const int kMicroSecondPerSecond = 1000 * 1000; 
private:
  int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
  return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}

inline double timeDiffence(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff / Timestamp::kMicroSecondPerSecond);
}

inline Timestamp addTime(Timestamp timestamp, double seconds)
{
  int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondPerSecond);
  return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}
}