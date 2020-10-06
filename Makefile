
CXXFLAGS+=-std=c++17
CXXFLAGS+=-g

LIBS+=-lstdc++
LIBS+=-lz

GCCVERSION=$(shell gcc -dumpversion)
ifeq ($(GCCVERSION),9)
else
LIBS+=-lstdc++fs
endif

wyag: GitRepository.cpp ConfigParser.cpp GitObject.cpp main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
