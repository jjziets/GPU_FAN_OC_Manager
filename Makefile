# Makefile for set_fan_curve

# Compiler settings - Can change to clang++ if preferred
CXX=g++
CXXFLAGS=-std=c++11 -O3 -Wall -Werror -Wextra -I/usr/local/cuda/include

# Linker settings
LDFLAGS=-L/usr/local/cuda/lib64 -lnvidia-ml

# Target executable name
TARGET=set_fan_curve

# Source files
SOURCES=set_fan_curve.cpp

# Object files
OBJECTS=$(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

clean:
	rm -f $(TARGET) $(OBJECTS)

install:
	cp $(TARGET) /usr/local/bin/

