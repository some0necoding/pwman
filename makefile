# this file simply compiles all source code
# Usage:
# make install			# installs pwman in /usr/local/bin/
# make clean			# removes pwman from /usr/local/bin

SHELL := /bin/bash

.SUFFIXES:
.SUFFIXES: .c .o .h

.PHONY := all help install clean

CC := gcc

CFLAGS := -lgpgme -lX11 -lpthread

COMMANDS := ./commands/source/psm_add.o  \
			./commands/source/psm_exit.o \
			./commands/source/psm_get.o  \
			./commands/source/psm_help.o \
			./commands/source/psm_rm.o   \
			./commands/source/psm_show.o 

UTILS := ./utils/source/clipboard.o \
		 ./utils/source/config.o    \
		 ./utils/source/console.o   \
		 ./utils/source/crypto.o    \
		 ./utils/source/fio.o		\
		 ./utils/source/input.o	  	\
		 ./utils/source/path.o

PWMAN := ./pwman.o
INIT := ./pwman-init.o

SYSTEM_PATH := /usr/local/bin

EXEC_NAME := $(SYSTEM_PATH)/$(PWMAN:%.o=%)
INIT_NAME := $(SYSTEM_PATH)/$(INIT:%.o=%)

all: help

help:
	@echo "Usage:"
	@echo "  run \"make install\" to install $(PWMAN:%.o=%)"
	@echo "  run \"make clean\" to remove $(PWMAN:%.o=%)"

install: $(EXEC_NAME) $(INIT_NAME)
	@echo "Done"

# Compiling pwman	
$(EXEC_NAME): $(COMMANDS) $(UTILS) $(PWMAN)
	@sudo $(CC) -o $(EXEC_NAME) $(COMMANDS) $(UTILS) $(PWMAN) $(CFLAGS)

# Compiling pwman-init
$(INIT_NAME): $(UTILS) $(INIT)
	@sudo $(CC) -o $(INIT_NAME) $(UTILS) $(INIT) $(CFLAGS)

pwman.o: ./utils/headers/input.h 	   \
		 ./utils/headers/fio.h 		   \
		 ./utils/headers/config.h 	   \
		 ./commands/headers/psm_add.h  \
		 ./commands/headers/psm_show.h \
		 ./commands/headers/psm_rm.h   \
		 ./commands/headers/psm_get.h  \
		 ./commands/headers/psm_help.h \
		 ./commands/headers/psm_exit.h

pwman-init.o: ./utils/headers/config.h \
			  ./utils/headers/crypto.h

# Commands build
commands/source/psm_add.o: ./utils/headers/input.h 	\
						   ./utils/headers/config.h \
						   ./utils/headers/crypto.h \
						   ./utils/headers/path.h	\
						   ./commands/headers/psm_add.h

commands/source/psm_exit.o: ./commands/headers/psm_exit.h

commands/source/psm_get.o: ./utils/headers/clipboard.h \
						   ./utils/headers/crypto.h    \
						   ./utils/headers/config.h    \
						   ./utils/headers/fio.h 	   \
						   ./utils/headers/path.h  	   \
						   ./commands/headers/psm_get.h 

commands/source/psm_help.o: ./commands/headers/psm_help.h

commands/source/psm_rm.o: ./utils/headers/config.h \
						  ./utils/headers/path.h   \
						  ./commands/headers/psm_rm.h

commands/source/psm_show.o: ./utils/headers/config.h \
						  	./utils/headers/path.h 	 \
						  	./commands/headers/psm_show.h

# Utils build
utils/source/clipboard.o: ./utils/headers/clipboard.h

utils/source/config.o: ./utils/headers/fio.h \
					   ./utils/headers/config.h

utils/source/console.o: ./utils/headers/console.h

utils/source/crypto.o: ./utils/headers/input.h \
					   ./utils/headers/crypto.h 

utils/source/fio.o: ./utils/headers/fio.h

utils/source/input.o: ./utils/headers/console.h \
					  ./utils/headers/input.h 

utils/source/path.o: ./utils/headers/path.h 

# Deleting bins and object files
clean:
	@echo "Cleaning up..."
	@sudo rm $(UTILS) $(COMMANDS) $(PWMAN) $(INIT)
	@sudo rm $(EXEC_NAME)
	@sudo rm $(INIT_NAME)
	@echo "Done"