CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2

TARGET = cpu

SRCS = main.cpp tinyfiledialogs.c

LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)

.PHONY: all clean