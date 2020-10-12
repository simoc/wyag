
CXXFLAGS+=-std=c++17
CXXFLAGS+=-g
CXXFLAGS+=-Wall

LIBS+=-lstdc++
LIBS+=-lz
LIBS+=-lcrypto

GCCVERSION=$(shell gcc -dumpversion)
ifeq ($(GCCVERSION),9)
else
LIBS+=-lstdc++fs
endif

wyag: GitRepository.cpp ConfigParser.cpp GitObject.cpp GitBlob.cpp main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
