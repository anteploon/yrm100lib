CC = gcc
SANITIZE_FLAGS = -fsanitize=undefined,signed-integer-overflow
ifeq ($(OS),Windows_NT)
SANITIZE_FLAGS =
endif
CFLAGS = -pedantic $(SANITIZE_FLAGS) -Wconversion -Wall -Wextra -Werror -fmax-errors=3 -O0
INCLUDES = -Isrc
TARGET = example
TEST_TARGET = test_example
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

OBJS = $(SRCS:.c=.o)
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
TEST_OBJS = $(TEST_SRCS:.c=.o)

ifeq ($(OS),Windows_NT)
WIN_OBJS = $(subst /,\,$(OBJS))
WIN_TEST_OBJS = $(subst /,\,$(TEST_OBJS))
WIN_CLEAN = $(sort $(WIN_OBJS) $(WIN_TEST_OBJS) $(TARGET).exe $(TEST_TARGET).exe)
else
CLEAN = $(sort $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET))
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-powershell -NoProfile -Command '$$files = "$(WIN_CLEAN)".Split(" "); if ($$files[0] -ne "") { Remove-Item -Force -ErrorAction SilentlyContinue -LiteralPath $$files }; exit 0'
else
	rm -f $(CLEAN)
endif

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TEST_TARGET) $(TEST_OBJS)

rebuild: clean all

.PHONY: all clean test rebuild
