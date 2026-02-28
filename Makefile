# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
LDFLAGS = -lncurses -lpthread

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = whiletrue

# Source files
SRCS = $(SRC_DIR)/main.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET)

# Clean up build artifacts
clean:
	rm -f $(SRC_DIR)/*.o $(TARGET)

.PHONY: all run clean
