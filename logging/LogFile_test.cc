#include "LogFile.h"
#include "Logging.h"

boost::scoped_ptr<jiangbo::LogFile> g_logFile;

void outputFunc(const char* msg, int len)
{
  g_logFile->append(msg, len);
}

void flushFunc()
{
  g_logFile->flush();
}

int main(int argc, char* argv[])
{
  char name[256];
  strncpy(name, argv[0], 256);
  g_logFile.reset(new jiangbo::LogFile(::basename(name), 256*1024));
  jiangbo::Logger::setOutput(outputFunc);
  jiangbo::Logger::setFlush(flushFunc);

  jiangbo::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

  for (int i = 0; i < 10000; ++i)
  {
    LOG_INFO << line;

    usleep(1000);
  }
}