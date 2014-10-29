CC=gcc
CXX=g++ -Wall
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs) -lserial

EXEC=exec
SRCS=main.cpp speedcontrol.cpp serial.cpp serialsim.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(EXEC) $(SIMEXEC)

debug: CPPFLAGS += -DDEBUG -g
debug: $(EXEC)
debug: $(SIMEXEC)

$(EXEC): $(OBJS)
	$(CXX) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)
	$(RM) $(EXEC)
	$(RM) $(SIMEXEC)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend