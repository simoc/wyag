
CXXFLAGS+=-std=c++17

wyag: GitRepository.cpp ConfigParser.cpp main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ -lstdc++
