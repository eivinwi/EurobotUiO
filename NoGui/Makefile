CC=gcc
CXX=g++ -Wall -std=c++11 -lpthread -D_ELPP_DEBUG_ERRORS
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs) -lserial -lzmq -pthread

EXEC=exec
SRCS=main.cpp motorcom.cpp serial.cpp serialsim.cpp poscontrol.cpp position.cpp goalposition.cpp rotation.cpp
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