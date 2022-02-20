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
# creating empty config/db files (this set of commands can be coded way better than this)
	@sudo mkdir config_files
	@sudo touch config_files/accounts.list
	@sudo touch config_files/passwords.list
	@sudo touch config_files/crypto.salt
	@sudo touch config_files/login.hash
	@sudo chmod ug=rwx config_files/*
# compiling source code and saving the bin executable in /usr/local/bin to make it runnable from terminal
	@sudo $(CC) -o /usr/local/bin/$(EXEC_NAME) $(MAIN_SRC) $(HEAD_SRC) $(FLAG_1) $(FLAG_2) $(FLAG_3)

clean:
	@echo "Cleaning up..."
# deleting bin executable from /usr/local/bin
	@sudo rm -f /usr/local/bin/$(EXEC_NAME)
# deleting config/db files used (this set of commands can be coded way better than this)
	@sudo rm -f config_files/accounts.list
	@sudo rm -f config_files/passwords.list
	@sudo rm -f config_files/crypto.salt
	@sudo rm -f config_files/login.hash
	@sudo rmdir config_files
	@echo "Done"