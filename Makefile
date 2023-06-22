
CC = cc
LD = cc
ALPHABET := abcdefghijklmnopqrstuvwxyz

ifeq ($(ALPHABET), abcdefghijklmnopqrstuvwxyz)
	VARIANT_CFLAGS :=
	VARIANT_SUFFIX :=
else
	VARIANT_CFLAGS := -D'BOMM_ALPHABET="$(ALPHABET)"'
	VARIANT_SUFFIX := $(addprefix -, $(ALPHABET))
endif

RELEASE_CFLAGS := $(CFLAGS) -O3 -std=c11 -pthread -pedantic -Wall -Werror -Wextra $(VARIANT_CFLAGS)
RELEASE_LDFLAGS := $(LDFLAGS) -lm -lpthread -ljansson
TEST_CFLAGS := $(CFLAGS) -std=c11 -pthread -pedantic -Wall -Werror -Wextra $(VARIANT_CFLAGS)
TEST_LDFLAGS := $(LDFLAGS) -lm -lpthread -ljansson -lcriterion

TARGET_EXEC := bomm$(VARIANT_SUFFIX)
BUILD_PATH := build
TARGET_PATH := $(BUILD_PATH)/$(TARGET_EXEC)

# Release dirs
RELEASE_SRC_PATH := src
RELEASE_OBJ_PATH := $(BUILD_PATH)/obj
RELEASE_OBJ_MAIN_PATH := $(RELEASE_OBJ_PATH)/main$(VARIANT_SUFFIX).o

# Test dirs
TEST_SRC_PATH := tests
TEST_OBJ_PATH := $(BUILD_PATH)/tests/obj
TEST_BIN_PATH := $(BUILD_PATH)/tests/bin

# List source and object files
RELEASE_SRCS := $(shell find $(RELEASE_SRC_PATH) -type f -name '*.c')
RELEASE_OBJS := $(subst $(RELEASE_SRC_PATH), $(RELEASE_OBJ_PATH), $(RELEASE_SRCS:.c=$(VARIANT_SUFFIX).o))

TEST_SRCS := $(shell find $(TEST_SRC_PATH) -type f -name '*.c')
TEST_OBJS := $(subst $(TEST_SRC_PATH), $(TEST_OBJ_PATH), $(TEST_SRCS:.c=$(VARIANT_SUFFIX).o))
TEST_BINS := $(subst $(TEST_SRC_PATH), $(TEST_BIN_PATH), $(TEST_SRCS:.c=$(VARIANT_SUFFIX)))

# Compile release task
$(RELEASE_OBJ_PATH)/%$(VARIANT_SUFFIX).o: $(RELEASE_SRC_PATH)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(RELEASE_CFLAGS)

# Link release task
$(TARGET_PATH): $(RELEASE_OBJS)
	mkdir -p $(dir $@)
	$(LD) $(RELEASE_OBJS) -o $@ $(RELEASE_LDFLAGS)

# Compile tests task
$(TEST_OBJ_PATH)/%$(VARIANT_SUFFIX).o: $(TEST_SRC_PATH)/%.c
	mkdir -p $(dir $@)
	$(CC) -c $^ -o $@ $(TEST_CFLAGS)

# Link tests task
$(TEST_BIN_PATH)/%$(VARIANT_SUFFIX): $(TEST_OBJ_PATH)/%$(VARIANT_SUFFIX).o $(filter-out $(RELEASE_OBJ_MAIN_PATH), $(RELEASE_OBJS))
	mkdir -p $(dir $@)
	$(LD) $^ -o $@ $(TEST_LDFLAGS)

build: $(TARGET_PATH)

run: $(TARGET_PATH)
	./$^ || true

test: $(TEST_BINS)
	EXIT_CODE=0; \
	for PATH in $^; do \
		echo "Test suite $$PATH"; \
		./$$PATH || EXIT_CODE=$$?; \
	done; \
	exit $$EXIT_CODE;

# Clean task
.PHONY: clean
clean:
	if [ -d $(BUILD_PATH) ]; then rm -r $(BUILD_PATH); fi

# Include the .d makefiles
-include $(DEPS)
