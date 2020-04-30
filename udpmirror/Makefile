CXXFLAGS += -Wall -ggdb

MPLX=mplx_server mplx_client

BINFILES = $(MPLX)

all: $(BINFILES)

common.o: common.cc common.h
udpsocket.o: udpsocket.cc udpsocket.h common.h
callerlist.o: callerlist.cc callerlist.h udpsocket.h common.h

$(DUP): LDLIBS+=-lboost_system -lpthread

$(MPLX): LDLIBS+=-lpthread

$(BINFILES): common.o udpsocket.o callerlist.o

mplx_client: LDLIBS+=-llo

clean:
	rm -f *~ $(BINFILES) *.o
