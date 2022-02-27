# this file simply compiles all source code
# Usage:
# make install			# installs pwman in /usr/local/bin/
# make clean			# removes pwman from /usr/local/bin

.PHONY := all help install create_needed_files create_bin_dir create_bin_files compile clean remove_exec delete_bin_files delete_bin_dir

CC := gcc

LIBS := sodium X11 pthread
CFLAGS := $(LIBS:%=-l%)

SRCS := $(wildcard *.c) $(wildcard headers_source/*.c)

SYSTEM_PATH := /usr/local/bin

BIN_FOLDER := binaries
BIN_FILES := accounts.list passwords.list crypto.salt login.hash

EXEC_NAME := pwman

all: help

help:
	@echo "Usage:"
	@echo "  run \"make install\" to install $(EXEC_NAME)"
	@echo "  run \"make clean\" to remove $(EXEC_NAME)"

install: create_needed_files compile
	@echo "Done"

create_needed_files: create_bin_dir create_bin_files

create_bin_dir:
	@echo "Creating needed files..."
	@sudo mkdir $(BIN_FOLDER)

create_bin_files: $(BIN_FILES)
	@sudo chmod a=rwx $(BIN_FOLDER)/*
	@echo "Needed files created"

$(BIN_FILES):
	@sudo touch $(BIN_FOLDER)/$@

# compiling source code and saving the bin executable in /usr/local/bin to make it easily runnable from terminal
compile:
	@echo "Compiling..."
	@sudo $(CC) -o $(SYSTEM_PATH)/$(EXEC_NAME) $(SRCS) $(CFLAGS)

# deleting bin executable from /usr/local/bin and deleting bin files
clean: remove_exec delete_bin_files delete_bin_dir
	@echo "Done"

remove_exec:
	@echo "Cleaning up..."
	@sudo rm $(SYSTEM_PATH)/$(EXEC_NAME)

delete_bin_files:
	@sudo rm $(BIN_FOLDER)/*

delete_bin_dir:
	@sudo rmdir $(BIN_FOLDER)