# Makefile for read_number (userspace C program)
# SPDX-License-Identifier: MIT

CC      ?= gcc
CFLAGS  ?= -Wall -Wextra -O2

TARGET  = read_number

all: $(TARGET)

$(TARGET): read_number.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

test: $(TARGET)
	@echo "=== Test 1: single digit ==="
	echo "7" | ./$(TARGET)
	@echo ""
	@echo "=== Test 2: two digits ==="
	echo "42" | ./$(TARGET)
	@echo ""
	@echo "=== Test 3: invalid input (letters) ==="
	echo "ab" | ./$(TARGET)
	@echo ""
	@echo "=== Test 4: invalid input (too many digits) ==="
	echo "123" | ./$(TARGET)

.PHONY: all clean test
