CXXFLAGS = -O0 -g -Wall -pthread
LDFlAGES = -lpthread -ljiangbo -L.
BASE_SRC = ../logging/Logging.cc ../logging/LoggingStream.cc ../thread/Thread.cc ../timestamp/TimeStamp.cc
JIANGBO_SRC = $(notdir $(BASE_SRC) $(LIB_SRC))
OBJECTS = $(patsubst %.cc,%.o,$(JIANGBO_SRC))

libjiangbo.a : $(BASE_SRC) $(LIB_SRC)
	g++ $(CXXFLAGS) -c $^
	ar rcs $@ $(OBJECTS)

$(BINARIES) : libjiangbo.a
	g++ $(CXXFLAGS) -o $@ $(filter %.cc, $^) $(LDFlAGES)

clean:
	rm -f $(BINARIES) *.o *.a core
