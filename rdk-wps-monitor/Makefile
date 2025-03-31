##########################################################################
# If not stated otherwise in this file or this component's LICENSE 
# file the following copyright and licenses apply:
#
# Copyright 2025 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c99 -D_GNU_SOURCE
LDFLAGS = -pthread

# Directories
SRC_DIR = source
INCLUDE_DIR = include
OBJ_DIR = obj
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
EXECUTABLE = $(BIN_DIR)/netlink-button-monitor

# Header files
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)

# Debug build flags
DEBUG_CFLAGS = -g -DDEBUG

.PHONY: all clean debug install uninstall

all: prepare $(EXECUTABLE)

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: all

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ -lrbus -lrtMessage -lrbuscore

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c -o $@ $<

prepare:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

install: all
	@echo "Installing netlink-button-monitor"
	@install -m 755 $(EXECUTABLE) /usr/local/bin/
	@echo "Installation complete"

uninstall:
	@echo "Uninstalling netlink-button-monitor"
	@rm -f /usr/local/bin/netlink-button-monitor
	@echo "Uninstallation complete"
