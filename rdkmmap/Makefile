/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.*/

# Makefile for MT7988 Register Access Tool

CC ?= gcc
CROSS_COMPILE ?=
TARGET = rdkmmap

# Set the cross compiler if cross-compiling
ifneq ($(CROSS_COMPILE),)
CC := $(CROSS_COMPILE)gcc
endif

# Directories
SRC_DIR = source
INCLUDE_DIR = include
BUILD_DIR = build

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -std=c99 -I$(INCLUDE_DIR)

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Object files - put them in the build directory
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(SRC_DIR)
	@mkdir -p $(INCLUDE_DIR)

# Build target
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Cross-compilation targets (cleans first to ensure proper architecture build)
arm: clean
	$(MAKE) CROSS_COMPILE=arm-linux-gnueabi-

aarch64: clean
	$(MAKE) CROSS_COMPILE=aarch64-linux-gnu-

# x86 target (32-bit)
x86: clean
	$(MAKE) CROSS_COMPILE=i686-linux-gnu-

# x86_64 target (64-bit)
x86_64: clean
	$(MAKE) CROSS_COMPILE=x86_64-linux-gnu-

# Install the binary to /usr/bin
install: $(TARGET)
	install -m 755 $(TARGET) /usr/bin/

# Clean up
clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)/*

.PHONY: all directories arm aarch64 x86 x86_64 install clean
