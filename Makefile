CC=g++
CXX=g++
RANLIB=ranlib

LIBSRC=VirtualMemory.cpp
HEADERS=MemoryConstants.h
LIBOBJ=$(LIBSRC:.cpp=.o)


INCS=-I.
CFLAGS = -Wall -std=c++11 -g -lpthread $(INCS)
CXXFLAGS = -Wall -std=c++11 -g -lpthread $(INCS)

UTHREADLIB = libVirtualMemory.a
TARGETS = $(UTHREADLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex4.tar
TARSRCS=$(LIBSRC) Makefile README

all: $(TARGETS)

$(TARGETS): $(LIBOBJ) $(HEADERS)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(UTHREADLIB) $(OBJ) $(LIBOBJ) *~ *core

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)