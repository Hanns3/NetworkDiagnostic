# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++11 -g

# Directories
SRC_DIR := src
BUILD_DIR := build

# Find all .cpp files in src/ and generate corresponding .o files in build/
SRC_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# Output executable
TARGET := my_traceroute

# Default rule
all: $(TARGET)

# Link object files into the final executable
$(TARGET): $(OBJ_FILES) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile each .cpp file into an .o file
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create necessary directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	rm my_traceroute

# Print variables for debugging
print:
	@echo "SRC_FILES: $(SRC_FILES)"
	@echo "OBJ_FILES: $(OBJ_FILES)"
	@echo "TARGET: $(TARGET)"

.PHONY: all run clean print
