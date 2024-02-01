# Makefile

CC := gcc
CFLAGS := -Iinclude
SRC_DIR := src
TEST_DIR := test
UNITY_DIR := $(TEST_DIR)/Unity/src

# List of source files
SRC_FILES := $(wildcard $(SRC_DIR)/*.c) $(UNITY_DIR)/unity.c
TEST_FILES := $(wildcard $(TEST_DIR)/*.c)

# List of object files
OBJ_FILES := $(SRC_FILES:.c=.o) $(TEST_FILES:.c=.o)

# Output executable
TARGET := test_rflink_util

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJ_FILES)

