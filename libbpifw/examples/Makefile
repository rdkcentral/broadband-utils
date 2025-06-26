PARENT_DIR := ..
include $(PARENT_DIR)/Makefile

# Override some settings for examples
EXAMPLE_CFLAGS := $(CFLAGS) -I$(PARENT_DIR)/$(INCDIR)
EXAMPLE_LDFLAGS := -L$(PARENT_DIR)/$(LIBDIR) -l$(PROJECT_NAME:lib%=%)$(BUILD_SUFFIX)

.PHONY: examples-only
examples-only: $(EXAMPLE_BINS)

$(BINDIR)/%: %.c | $(BINDIR)
	@echo "Building example: $@"
	$(CC) $(EXAMPLE_CFLAGS) -o $@ $< $(EXAMPLE_LDFLAGS) $(LIBS)
