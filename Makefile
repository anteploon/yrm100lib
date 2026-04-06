CC = gcc
SANITIZE_FLAGS = -fsanitize=undefined,signed-integer-overflow
ifeq ($(OS),Windows_NT)
SANITIZE_FLAGS =
endif
CFLAGS = -pedantic $(SANITIZE_FLAGS) -Wconversion -Wall -Wextra -Werror -fmax-errors=3 -O0
INCLUDES = -Isrc
TARGET = example
TEST_TARGET = test_example
BUILD_DIR = build
TARGET_PATH = $(BUILD_DIR)/$(TARGET)
TEST_TARGET_PATH = $(BUILD_DIR)/$(TEST_TARGET)
SRCS = src/example.c \
	src/yrm100/yrm100_command.c \
	src/yrm100/yrm100_error.c \
	src/yrm100/yrm100_frame.c \
	src/yrm100/yrm100_parse.c \
	src/yrm100/yrm100_serial.c \
	src/yrm100/yrm100_string.c \
	src/yrm100/yrm100_util.c \
	src/yrm100/yrm100_print.c \
	src/yrm100/yrm100.c

OBJS = $(SRCS:%.c=$(BUILD_DIR)/%.o)
TEST_SRCS = tests/test_example.c \
	tests/test_serial.c \
	tests/test_string.c \
	src/yrm100/yrm100_command.c \
	src/yrm100/yrm100_error.c \
	src/yrm100/yrm100_frame.c \
	src/yrm100/yrm100_parse.c \
	src/yrm100/yrm100_string.c \
	src/yrm100/yrm100_util.c \
	src/yrm100/yrm100_print.c \
	src/yrm100/yrm100.c
TEST_OBJS = $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)

ifeq ($(OS),Windows_NT)
WIN_OBJS = $(subst /,\,$(OBJS))
WIN_TEST_OBJS = $(subst /,\,$(TEST_OBJS))
WIN_CLEAN = $(BUILD_DIR)
else
CLEAN = $(BUILD_DIR)
endif

all: $(TARGET_PATH)

$(TARGET_PATH): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-powershell -NoProfile -Command 'if (Test-Path "$(WIN_CLEAN)") { Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -LiteralPath "$(WIN_CLEAN)" }; exit 0'
else
	rm -rf $(CLEAN)
endif

test: $(TEST_TARGET_PATH)
	@./$(TEST_TARGET_PATH)

$(TEST_TARGET_PATH): $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(TEST_OBJS)

rebuild: clean all

.PHONY: all clean test rebuild
