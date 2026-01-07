CC = gcc
CFLAGS = -pedantic -fsanitize=undefined,signed-integer-overflow -Wconversion -Wall -Wextra -Werror -fmax-errors=3 -O0
INCLUDES = -I.
TARGET = example
TEST_TARGET = test_example
SRCS = example.c \
	yrm100/yrm100_command.c \
	yrm100/yrm100_error.c \
	yrm100/yrm100_frame.c \
	yrm100/yrm100_parse.c \
	yrm100/yrm100_serial.c \
	yrm100/yrm100_string.c \
	yrm100/yrm100_util.c \
	yrm100/yrm100_print.c \
	yrm100/yrm100.c

OBJS = $(SRCS:.c=.o)
TEST_SRCS = ../tests/test_example.c \
	../tests/test_serial.c \
	yrm100/yrm100_command.c \
	yrm100/yrm100_error.c \
	yrm100/yrm100_frame.c \
	yrm100/yrm100_parse.c \
	yrm100/yrm100_string.c \
	yrm100/yrm100_util.c \
	yrm100/yrm100_print.c \
	yrm100/yrm100.c
TEST_OBJS = $(TEST_SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)

test: $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(TEST_TARGET) $(TEST_OBJS)

rebuild: clean all

.PHONY: all clean test rebuild
