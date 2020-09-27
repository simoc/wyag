
CXXFLAGS+=-std=c++17

LIBS+=-lstdc++

GCCVERSION=$(shell gcc -dumpversion)
ifeq ($(GCCVERSION),9)
else
LIBS+=-lstdc++fs
endif

wyag: GitRepository.cpp ConfigParser.cpp main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)
