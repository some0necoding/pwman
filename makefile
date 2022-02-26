# this file simply compiles all source code
# Usage:
# make install			# installs passman in /usr/local/bin/
# make clean			# removes passman from /usr/local/bin

.PHONY = all help install create_bins delete_bins clean

CC = gcc

CFLAGS := -lsodium -lX11 -lpthread

SRCS := $(wildcard *.c) $(wildcard headers_source/*.c)

PATH = /usr/local/bin

BIN_FOLDER = binaries
BINS := accounts.list passwords.list crypto.salt login.hash

EXEC_NAME = pwman

all: help

help:
	@echo "Usage:"
	@echo "  run \"make install\" to install $(EXEC_NAME)"
	@echo "  run \"make clean\" to remove $(EXEC_NAME)"

install:
# creating empty bin files
	@sudo mkdir $(BIN_FOLDER)
	@create_bins
	@sudo chmod a=rwx $(BIN_FOLDER)/*

# compiling source code and saving the bin executable in /usr/local/bin to make it runnable from terminal
	@echo "Compiling..."
	@sudo $(CC) -o $(PATH)/$(EXEC_NAME) $(SRCS) $(CFLAGS)
	@echo "Done"

create_bins: $(BINS)
# creating empty bin files
	@echo "Creating needed files..."
	@sudo touch $(BIN_FOLDER)/$@

delete_bins: $(BINS)
# deleting bin files
	@echo "Deleting bin files..."
	@sudo rm -f $(BIN_FOLDER)/$@

clean:
# deleting bin executable from /usr/local/bin
	@echo "Cleaning up..."
	@sudo rm -f $(PATH)/$(EXEC_NAME)
	@delete_bins
	@sudo rmdir $(BIN_FOLDER)
	@echo "Done"