#include "Logging.h"
#include "LogFile.h"

#include <stdio.h>

long g_total;
FILE *g_file;
boost::scoped_ptr<jiangbo::LogFile> g_logFile;

void dummyOutput(const char *msg, int len){
  g_total += len;
  if(g_file){
    fwrite(msg, 1, len, g_file);
  }else if(g_logFile){
    g_logFile->append(msg, len);
  }
}

void bench(){
  jiangbo::Logger::setOutput(dummyOutput);
  jiangbo::Timestamp start(jiangbo::Timestamp::now());

  g_total = 0;

  const int batch = 1e6;
  const bool kLongLog = false;

  jiangbo::string empty = " ";
  jiangbo::string longstr(3000, 'x');
  longstr += ' ';

  for(int i = 0; i < batch; ++ i){
    LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
             << (kLongLog ? longstr : empty)
             << i;
  }
  jiangbo::Timestamp end(jiangbo::Timestamp::now());
  double seconds = jiangbo::timeDiffence(end, start);

  printf("%f seconds, %ld bytes, %.2f msg/s, %.2f MiB/s\n",
         seconds, g_total, batch / seconds, g_total / seconds / 1024 / 1024);

}

int main()
{
  getppid();
  LOG_TRACE << "trace";
  LOG_DEBUG << "debug";
  LOG_INFO << "Hello";
  LOG_WARN << "World";
  LOG_ERROR << "Error";
  LOG_INFO << sizeof(jiangbo::Logger);
  LOG_INFO << sizeof(jiangbo::LoggingStream);
  LOG_INFO << sizeof(jiangbo::Fmt);
  LOG_INFO << sizeof(jiangbo::LoggingStream::Buffer);

  bench();

  char buffer[64*1024];

  g_file = fopen("/dev/null", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench();
  fclose(g_file);

  g_file = fopen("/tmp/log", "w");
  setbuffer(g_file, buffer, sizeof buffer);
  bench();
  fclose(g_file);

  g_file = NULL;
  g_logFile.reset(new jiangbo::LogFile("test_log", 500*1000*1000));
  bench();

  g_logFile.reset(new jiangbo::LogFile("test_log_mt", 500*1000*1000, true));
  bench();
  g_logFile.reset();
}