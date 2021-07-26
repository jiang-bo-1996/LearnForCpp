  
g++ LoggingStream_test.cc LoggingStream.cc -o test -Wall -g -O0 -DBOOST_TEST_DYN_LINK -lboost_unit_test_framework

g++ Logging_test.cc Logging.cc LoggingStream.cc LogFile.cc ../TimeStamp.cc ../Thread.cc -O2 -DNDEBUG -I.. -o test_log -Wall  -g -lpthread
g++ Logging_test.cc Logging.cc LoggingStream.cc LogFile.cc ../TimeStamp.cc ../Thread.cc -O0 -I.. -o test_log-dbg -Wall  -g -lpthread

g++ Logging_test.cc Logging.cc LoggingStream.cc LogFile.cc ../TimeStamp.cc ../Thread.cc -O2 -DNDEBUG -I.. -o file_test -g -lpthread
g++ Logging_test.cc Logging.cc LoggingStream.cc LogFile.cc ../TimeStamp.cc ../Thread.cc -O0 -I.. -o file_test-dbg -g -lpthread
