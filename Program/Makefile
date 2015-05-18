CC=gcc-4.8
CXX=g++-4.8 -Wall -std=c++11 -lpthread
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs) -lserial -lzmq -pthread -lyaml-cpp -lboost_program_options
#`sdl-config --libs` -lSDL_mixer

EXEC=control
SRCS=main.cpp motorcom.cpp dynacom.cpp serial.cpp serialsim.cpp poscontrol.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(EXEC)

debug: CPPFLAGS += -DDEBUG -g
debug: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) $(EXEC)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend
