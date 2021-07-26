#include "../reactor/EventLoop.h"
#include "../timestamp/TimeStamp.h"
#include "../logging/Logging.h"
#include "../reactor/TimerId.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>

using namespace jiangbo;

const double diff = 1e-3, interval = 1e-4;

int numoftimer = 0;

EventLoop *g_loop;

void timeout(){
}

void addTimer(){
  static Timestamp launchTime(Timestamp::now());
  launchTime = addTime(launchTime, 1e-4);
  Timestamp now(Timestamp::now());
  if(now.microSecondsSinceEpoch() - launchTime.microSecondsSinceEpoch() <= diff){
    for(int i = 0; i < 1e5; ++ i){
      g_loop->runEvery(interval, boost::bind(timeout));
    }
    numoftimer += 1e5;
  }else{
    LOG_DEBUG << "When timeout error is greater than 1000ms, The number of timer is " << numoftimer << " ;";
    g_loop->quit();
  }
}


int main(){
  EventLoop loop;
  g_loop = &loop;
  g_loop->runAfter(interval, boost::bind(addTimer));
  g_loop->loop();
  return 0;
}

