CC=gcc
CXX=g++ -Wall
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs) -lserial

EXEC=exec
SRCS=main.cpp serial.cpp speedcontrol.cpp serialsim.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(EXEC)

sim: SRCS += serialsim.cpp
sim: EXEC = sim
sim: $(EXEC)

debug: CPPFLAGS += -DDEBUG -g
debug: EXEC = debug
debug: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend