# SPDX-License-Identifier: GPL-2.0
#
# Makefile for the memory_hook kernel module

obj-m += memory_hook.o

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod memory_hook.ko

uninstall:
	sudo rmmod memory_hook

test: all
	@echo "--- Loading module ---"
	sudo insmod memory_hook.ko || true
	@echo "--- Testing (output should be replaced) ---"
	echo "42" | ./test_read
	@echo "--- Unloading module ---"
	sudo rmmod memory_hook
	@echo "--- Testing (output should be normal) ---"
	echo "42" | ./test_read

.PHONY: all clean install uninstall test
