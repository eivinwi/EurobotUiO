CC=gcc
CXX=g++ -Wall -std=c++11 -lpthread
RM=rm -f
CPPFLAGS=-g $(shell root-config --cflags)
LDFLAGS=-g $(shell root-config --ldflags)
LDLIBS=$(shell root-config --libs) -lserial -lzmq -pthread 
#`sdl-config --libs` -lSDL_mixer

EXEC=exec
SRCS=main.cpp motorcom.cpp liftcom.cpp serial.cpp serialsim.cpp poscontrol.cpp position.cpp goalposition.cpp rotation.cpp
# sound.cpp
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