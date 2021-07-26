
#g++ test1.cc EventLoop.cc Channel.cc Epoller.cc TimerQueue.cc Timer.cc ../thread/Thread.cc ../TimeStamp.o ../Logging.o ../LoggingStream.o -lpthread -o test1.o
LIB_SRC = Channel.cc EventLoop.cc Epoller.cc\
          Timer.cc TimerQueue.cc EventLoopThread.cc\
          Socketops.cc Socket.cc InetAddress.cc\
          Acceptor.cc TcpServer.cc TcpConnection.cc\
          EventLoopThreadPool.cc Buffer.cc Connector.cc\
          TcpClient.cc
BINARIES = test1 test2 test3 test4 test5 test6 test7 test8

all : $(BINARIES)

include ../reactor_lib.mk

test1:test1.cc
test2:test2.cc
test3:test3.cc
test4:test4.cc
test5:test5.cc
test6:test6.cc
test7:test7.cc
test8:test8.cc