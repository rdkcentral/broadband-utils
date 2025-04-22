# Makefile for MAC Address Generator with static linking for aarch64

# Default compiler
CXX_X86 = g++
# Use complete static linking for aarch64 to avoid any library compatibility issues
CXX_AARCH64 = aarch64-linux-gnu-g++ -static

# Default architecture is x86
ARCH ?= x86

# Set compiler based on architecture
ifeq ($(ARCH),x86)
    CXX = $(CXX_X86)
    TARGET = assign_mac_x86
    ARCH_BUILD_DIR = build/x86
else ifeq ($(ARCH),aarch64)
    CXX = $(CXX_AARCH64)
    TARGET = assign_mac_aarch64
    ARCH_BUILD_DIR = build/aarch64
    CXXFLAGS_EXTRA = -DAARCH64_BUILD
else
    $(error Invalid architecture specified. Use ARCH=x86 or ARCH=aarch64)
endif

# Directories
SRC_DIR = source
INC_DIR = include
OUTPUT_DIR = bin

# Source files (find all .cpp files in source directory)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
# Object files (architecture-specific build directory)
OBJS = $(patsubst $(SRC_DIR)/%.cpp,$(ARCH_BUILD_DIR)/%.o,$(SRCS))
# Header files
HEADERS = $(wildcard $(INC_DIR)/*.h)

# Compiler flags
CXXFLAGS = -std=c++14 -Wall -Wextra -Werror -pedantic -Wconversion -Wshadow -Wcast-qual -Wcast-align -Wdouble-promotion -Wformat=2 -Wuninitialized -Wnull-dereference -O2 -I$(INC_DIR) $(CXXFLAGS_EXTRA)

# Create directories if they don't exist
$(shell mkdir -p $(ARCH_BUILD_DIR) $(OUTPUT_DIR))

# Default target
all: $(TARGET)

# Pattern rule for object files
$(ARCH_BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	@echo "Compiling $< for $(ARCH) architecture..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Target for x86
assign_mac_x86: $(OBJS)
	@echo "Building for x86 architecture..."
	$(CXX_X86) $(CXXFLAGS) $(OBJS) -o $(OUTPUT_DIR)/$@
	@echo "Note: Will use ./serial_number.txt for x86 build"

# Target for aarch64
assign_mac_aarch64: $(OBJS)
	@echo "Building for aarch64 architecture with full static linking..."
	$(CXX_AARCH64) $(CXXFLAGS) $(OBJS) -o $(OUTPUT_DIR)/$@
	@echo "Note: Will use /nvram/serial_number.txt for aarch64 build"

# Build for both architectures
both: 
	$(MAKE) clean
	$(MAKE) ARCH=x86
	$(MAKE) clean
	$(MAKE) ARCH=aarch64

# Clean build files for current architecture
clean:
	@echo "Cleaning build files for $(ARCH) architecture..."
	rm -rf $(ARCH_BUILD_DIR)

# Clean all build files
clean-all:
	@echo "Cleaning all build files..."
	rm -f $(OUTPUT_DIR)/assign_mac_x86 $(OUTPUT_DIR)/assign_mac_aarch64
	rm -rf build

# Help information
help:
	@echo "MAC Address Generator Makefile"
	@echo ""
	@echo "Usage:"
	@echo "  make              - Build for x86 (default)"
	@echo "  make ARCH=x86     - Build for x86"
	@echo "  make ARCH=aarch64 - Build for aarch64"
	@echo "  make both         - Build for both architectures"
	@echo "  make clean        - Remove built files for current architecture"
	@echo "  make clean-all    - Remove all built files"
	@echo "  make help         - Display this help message"

.PHONY: all both clean clean-all help
