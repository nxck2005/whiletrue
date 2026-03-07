# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O3
LDFLAGS = -lncursesw -lpthread

# Installation paths
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share/cybergrind

# Directories
SRC_DIR = src
BUILD_DIR = build
TARGET = $(BUILD_DIR)/cybergrind

# Source files
SRCS = $(SRC_DIR)/main.cpp $(SRC_DIR)/game.cpp $(SRC_DIR)/renderer.cpp $(SRC_DIR)/utils.cpp $(SRC_DIR)/input_handler.cpp
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRCS))

# Add DATA_DIR to flags
CXXFLAGS += -DDATA_DIR=\"$(DATADIR)/data\"

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

# Add a new software upgrade to the data/upgrades.json
upgrade:
	python3 tools/add_upgrade.py

# View all software upgrades
list-upgrades:
	python3 tools/view_upgrades.py

# Install the game
install: all
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(DATADIR)/data
	install -m 755 $(TARGET) $(DESTDIR)$(BINDIR)/cybergrind
	cp -r data/* $(DESTDIR)$(DATADIR)/data/

# Uninstall the game
uninstall:
	rm -f $(DESTDIR)$(BINDIR)/cybergrind
	rm -rf $(DESTDIR)$(DATADIR)

# Clean up build artifacts
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean install uninstall
