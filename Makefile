CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc

LIB_SRC := src/avl_tree.cpp src/rb_tree.cpp
LIB_OBJ := $(LIB_SRC:src/%.cpp=build/%.o)

.PHONY: all test run demo clean

all: bin/arvores bin/testes bin/demo

build/%.o: src/%.cpp | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

bin/arvores: src/main.cpp $(LIB_OBJ) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

bin/demo: src/demo.cpp $(LIB_OBJ) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

bin/testes: tests/tests.cpp $(LIB_OBJ) | bin
	$(CXX) $(CXXFLAGS) $^ -o $@

build bin:
	mkdir -p $@

test: bin/testes
	./bin/testes

run: bin/arvores
	./bin/arvores

demo: bin/demo
	./bin/demo

clean:
	rm -rf build bin
