PARENT_DIR := ..
include $(PARENT_DIR)/Makefile

# Test framework (simple assertion-based)
TEST_CFLAGS := $(CFLAGS) -I$(PARENT_DIR)/$(INCDIR) -DTESTING
TEST_LDFLAGS := -L$(PARENT_DIR)/$(LIBDIR) -l$(PROJECT_NAME:lib%=%)$(BUILD_SUFFIX)

.PHONY: tests-only
tests-only: $(TEST_BIN)

.PHONY: run
run: $(TEST_BIN)
	@echo "Running tests..."
	@./$(TEST_BIN)

