CXXFLAGS += -Wall

MPLX=mplx_server mplx_client

BINFILES = $(MPLX)

all: $(BINFILES)

common.o: common.cc common.h
udpsocket.o: udpsocket.cc udpsocket.h common.h

$(DUP): LDLIBS+=-lboost_system -lpthread

$(MPLX): LDLIBS+=-lpthread

$(BINFILES): common.o udpsocket.o

clean:
	rm -f *~ $(BINFILES) *.o
