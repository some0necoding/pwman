# this file simply compiles all source code
# Usage:
# make install			# installs pwman in /usr/local/bin/
# make clean			# removes pwman from /usr/local/bin

.PHONY := all help install create_needed_files create_bin_dir create_bin_files compile clean remove_exec delete_bin_files delete_bin_dir

CC := gcc

LIBS := gpgme X11 pthread
CFLAGS := $(LIBS:%=-l%)

COMMANDS_SRCS := $(wildcard commands/source/*.c) 
UTILS_SRCS := $(wildcard utils/source/*.c)
PWMAN_SRC := pwman.c
INIT_SRC := pwman-init.c

SYSTEM_PATH := /usr/local/bin

EXEC_NAME := $(PWMAN_SRC:%.c=%)
INIT_NAME := $(INIT_SRC:%.c=%)

all: help

help:
	@echo "Usage:"
	@echo "  run \"make install\" to install $(EXEC_NAME)"
	@echo "  run \"make clean\" to remove $(EXEC_NAME)"

install: check-x11 compile
	@echo "Done"

check-x11:
# 	check if x11 is installed

# compiling source code and saving the bin executable in /usr/local/bin to make it easily runnable from terminal
compile:
	@echo "Compiling..."
	@sudo $(CC) -o $(SYSTEM_PATH)/$(EXEC_NAME) $(COMMANDS_SRCS) $(UTILS_SRCS) $(PWMAN_SRC) $(CFLAGS)
	@sudo $(CC) -o $(SYSTEM_PATH)/$(INIT_NAME) $(UTILS_SRCS) $(INIT_SRC) $(CFLAGS)

# deleting bin executable from /usr/local/bin and deleting bin files
clean: remove_exec
	@echo "Done"

remove_exec:
	@echo "Cleaning up..."
	@sudo rm $(SYSTEM_PATH)/$(EXEC_NAME)
	@sudo rm $(SYSTEM_PATH)/$(INIT_NAME)