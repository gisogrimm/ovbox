CXXFLAGS += -Wall

BINFILES = headtracker

all: $(BINFILES)

headtracker: LDLIBS+=-llo -ltascardmx -ltascar

clean:
	rm -f *~ $(BINFILES) *.o
