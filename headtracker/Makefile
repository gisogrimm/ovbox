CXXFLAGS += -Wall

BINFILES = headtracker

all: $(BINFILES)

headtracker: LDLIBS+=-llo

clean:
	rm -f *~ $(BINFILES) *.o

clangformat:
	clang-format-9 -i $(wildcard *.cc) $(wildcard *.h)
