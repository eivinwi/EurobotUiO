CC=gcc
CXX=g++ -Wall -std=c++11
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs) -lzmq

EXEC=testclient
SRCS=testclient.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(EXEC)

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