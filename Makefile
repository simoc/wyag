
CXXFLAGS+=-std=c++17
CXXFLAGS+=-g
CXXFLAGS+=-Wall
CXXFLAGS+=-Isha1

LIBS+=-lstdc++
LIBS+=-lz

GCCVERSION=$(shell gcc -dumpversion)
ifeq ($(GCCVERSION),9)
else
LIBS+=-lstdc++fs
endif

wyag: GitRepository.cpp ConfigParser.cpp GitObject.cpp GitBlob.cpp GitCommit.cpp GitTree.cpp GitTag.cpp main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
