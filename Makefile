CXX ?= clang++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Wno-unused-parameter
#CXXFLAGS += -g -O0 -DDEBUG
CPPFLAGS += -Isource -Ishared
LDLIBS = -lz

SOURCES := $(shell find source shared -name '*.cpp')
OBJECTS := $(SOURCES:%.cpp=build/obj/%.o)
HEADERS := $(shell find source shared -name '*.h')

all: build/FLWorldRender

build/FLWorldRender: $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDLIBS)

build/obj/%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -rf build

.PHONY: all clean
