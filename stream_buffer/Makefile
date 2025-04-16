CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O3 -pthread
DEBUG_FLAGS = -g -DDEBUG

# Directories
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = test

# Source directories
SRC_DIRS = $(SRC_DIR) \
           $(SRC_DIR)/core \
           $(SRC_DIR)/network \
           $(SRC_DIR)/processing \
           $(SRC_DIR)/utils

# Include paths
INCLUDES = -I$(INC_DIR)

# Find all source files
SOURCES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
# Main source file
MAIN = main.cpp

# Object files
OBJECTS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
MAIN_OBJ = $(BUILD_DIR)/main.o

# Target executable
TARGET = $(BUILD_DIR)/stream_buffer

# Test sources and objects
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJECTS = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/test/%.o,$(TEST_SOURCES))
TEST_TARGET = $(BUILD_DIR)/run_tests

# Targets
.PHONY: all clean debug test dirs

all: dirs $(TARGET)

# Create necessary build directories
dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(foreach dir,$(SRC_DIRS),$(BUILD_DIR)/$(dir))
	@mkdir -p $(BUILD_DIR)/test

# Compile source files
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS) $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Main file
$(MAIN_OBJ): $(MAIN)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Test build
test: dirs $(OBJECTS) $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(filter-out $(MAIN_OBJ), $(OBJECTS)) $(TEST_OBJECTS) -o $(TEST_TARGET)
	$(TEST_TARGET)

# Debug build
debug: CXXFLAGS += $(DEBUG_FLAGS)
debug: all

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
