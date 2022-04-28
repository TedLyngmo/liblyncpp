CPPS = $(wildcard example*.cpp)
OBJS = $(CPPS:.cpp=.o)
EXES = $(CPPS:.cpp=)

CVER := -std=c11
CXXVER := -std=c++14

OPTS := -O3 -I../include -Wall -Wextra -pedantic -pedantic-errors

CPPHEADERS = $(wildcard *.hpp)
CHEADERS = $(wildcard *.h)

all : $(EXES)

%: %.o ../include/lyn/timer_queue.hpp
	$(CXX) $(CXXVER) $(OPTS) -o $@ $< -pthread

$(OBJS): %.o : %.cpp $(CPPHEADERS) Makefile
	$(CXX) $(CXXVER) $(OPTS) -c -o $@ $< -pthread

format:
	clang-format -i *.hpp *.cpp

clean:
	rm -f $(EXES) $(OBJS)
