VERSION=0.2

CXXFLAGS += -Wall -std=c++11 -O3

MPLX=roomservice ovboxclient devconfigclient getmacaddr

BINFILES = $(MPLX)

GITMODIFIED=$(shell test -z "`git status --porcelain -uno`" || echo "-modified")
COMMITHASH=$(shell git log -1 --abbrev=7 --pretty='format:%h')

FULLVERSION=$(VERSION)-$(COMMITHASH)$(GITMODIFIED)

CXXFLAGS += -DOVBOXVERSION="\"$(FULLVERSION)\""

all: $(BINFILES) jackwaitforport testhwjitter

common.o: common.cc common.h
udpsocket.o: udpsocket.cc udpsocket.h common.h
callerlist.o: callerlist.cc callerlist.h udpsocket.h common.h
roomservice ovboxclient devconfigclient: LDLIBS = -lcurl
devconfigclient: RSJparser.tcc


$(DUP): LDLIBS+=-lboost_system -lpthread

$(MPLX): LDLIBS+=-lpthread

$(BINFILES): common.o udpsocket.o callerlist.o

jackwaitforport: LDLIBS+=-ljack

devconfigclient: LDLIBS+=-lasound

clean:
	rm -f *~ $(BINFILES) *.o

clangformat:
	clang-format-9 -i $(wildcard *.cc) $(wildcard *.h)

