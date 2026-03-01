# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O3
LDFLAGS = -lncurses -lpthread

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/whiletrue

# Source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/game.cpp $(SRC_DIR)/renderer.cpp $(SRC_DIR)/utils.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files to object files in the build directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the game
run: all
	./$(TARGET)

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean
