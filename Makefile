# this file simply compiles all source code
# Usage:
# make install			# installs passman in /usr/local/bin/
# make clean			# removes passman from /usr/local/bin

.PHONY = all help compile clean

CC = gcc

FLAG_1 = -lsodium
FLAG_2 = -lX11
FLAG_3 = -lpthread

MAIN_SRC = passwordmanager.c
HEAD_SRC = headers_source/*.c

EXEC_NAME = passman

all: help

help:
	@echo "Usage:"
	@echo "  run \"make install\" to install passman"
	@echo "  run \"make clean\" to remove passman"

install:
	@sudo $(CC) -o /usr/local/bin/$(EXEC_NAME) $(MAIN_SRC) $(HEAD_SRC) $(FLAG_1) $(FLAG_2) $(FLAG_3)

clean:
	@echo "Cleaning up..."
	@sudo rm -f /usr/local/bin/$(EXEC_NAME)
	@echo "Done"