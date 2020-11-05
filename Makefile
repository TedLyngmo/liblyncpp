EXE := example_usage

CVER := -std=c11
CXXVER := -std=c++20

OPTS := -Wall -Wextra -pedantic -pedantic-errors
LIBS := -pthread

CPPHEADERS = $(wildcard *.hpp)
CHEADERS = $(wildcard *.h)

all : $(EXE)

$(EXE) : $(EXE).o
	$(CXX) $(CXXVER) $(OPTS) -o $@ $< $(LIBS)

%.o : %.cpp $(CPPHEADERS) Makefile
	$(CXX) $(CXXVER) $(OPTS) -c -o $@ $<

format:
	clang-format -i *.hpp *.cpp

clean:
	rm -f $(EXE) *.o
