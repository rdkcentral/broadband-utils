PROJECT_NAME := libfwbpi
VERSION := 1.0.0

CC := gcc
AR := ar
RANLIB := ranlib

# Directories
SRCDIR := src
INCDIR := include
BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
LIBDIR := $(BUILDDIR)/lib
BINDIR := $(BUILDDIR)/bin

# Compiler flags - Fixed for compatibility
CFLAGS := -Wall -Wextra -std=c99 -fPIC
CFLAGS += -I$(INCDIR) -I$(SRCDIR)
CFLAGS += -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS_DEBUG := $(CFLAGS) -g -DDEBUG -O0 -Werror
CFLAGS_RELEASE := $(CFLAGS) -O2 -DNDEBUG

# Build mode
BUILD_MODE ?= release
ifeq ($(BUILD_MODE),debug)
    CFLAGS += $(CFLAGS_DEBUG)
else
    CFLAGS += $(CFLAGS_RELEASE)
endif

# Library files
STATIC_LIB := $(LIBDIR)/$(PROJECT_NAME).a
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
HEADERS := $(wildcard $(INCDIR)/*.h)

.PHONY: all clean static

all: static

static: $(STATIC_LIB)

$(STATIC_LIB): $(OBJECTS) | $(LIBDIR)
	@echo "Creating static library: $@"
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR) $(LIBDIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILDDIR)

.PHONY: help
help:
	@echo "Available targets:"
	@echo "  all     - Build static library"
	@echo "  static  - Build static library only"  
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build debug version"
