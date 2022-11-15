# this file simply compiles all source code
# Usage:
# make install			# installs pwman in /usr/local/bin/
# make clean			# removes pwman from /usr/local/bin

.PHONY := all help install check-x11 clean

CC := gcc

PWMAN_CFLAGS := -lgpgme -lX11 -lpthread
INIT_CFLAGS := -lgpgme

COMMANDS := commands/source/psm_add.o  \
			commands/source/psm_exit.o \
			commands/source/psm_get.o  \
			commands/source/psm_help.o \
			commands/source/psm_rm.o   \
			commands/source/psm_show.o 

UTILS := utils/source/clipboard.o \
		 utils/source/config.o    \
		 utils/source/console.o   \
		 utils/source/crypto.o    \
		 utils/source/fio.o		  \
		 utils/source/input.o	  \
		 utils/source/path.o

PWMAN := pwman.o
INIT := pwman-init.o

EXEC_NAME := $(PWMAN:%.c=%)
INIT_NAME := $(INIT:%.c=%)

SYSTEM_PATH := /usr/local/bin

all: help

help:
	@echo "Usage:"
	@echo "  run \"make install\" to install $(EXEC_NAME)"
	@echo "  run \"make clean\" to remove $(EXEC_NAME)"

install: check-x11 $(SYSTEM_PATH)/$(EXEC_NAME) $(SYSTEM_PATH)/$(INIT_NAME)
	@echo "Done"

check-x11:
# 	check if x11 is installed

# Compiling pwman	
$(SYSTEM_PATH)/$(EXEC_NAME): $(COMMANDS) $(UTILS) $(PWMAN)
	@sudo $(CC) -o $(SYSTEM_PATH)/$(EXEC_NAME) $(COMMANDS) $(UTILS) $(PWMAN) $(PWMAN_CFLAGS)

# Compiling pwman-init
$(SYSTEM_PATH)/$(INIT_NAME): $(UTILS) $(INIT)
	@sudo $(CC) -o $(SYSTEM_PATH)/$(INIT_NAME) $(UTILS) $(INIT) $(INIT_CFLAGS)

# Commands build
commands/source/psm_add.o: utils/headers/input.h 	  \
						   utils/headers/config.h 	  \ 
						   utils/headers/crypto.h 	  \
						   utils/headers/path.h		  \
						   commands/headers/psm_add.h

commands/source/psm_exit.o: commands/headers/psm_exit.h

commands/source/psm_get.o: utils/headers/clipboard.h \
						   utils/headers/crypto.h 	 \
						   utils/headers/config.h 	 \
						   utils/headers/fio.h 	 	 \
						   utils/headers/path.h		 \
						   commands/headers/psm_get.h 

commands/source/psm_help.o: commands/headers/psm_help.o

commands/source/psm_rm.o: utils/headers/config.h   	 \
						  utils/headers/path.h 		 \
						  commands/headers/psm_rm.h

commands/source/psm_show.o: utils/headers/config.h   \
						  	utils/headers/path.h 	 \
						  	commands/headers/psm_show.h

# Utils build
utils/source/clipboard.o: utils/headers/clipboard.h

utils/source/config.o: utils/headers/fio.h \
					   utils/headers/config.h

utils/source/console.o: utils/headers/console.h

utils/source/crypto.o: utils/headers/input.h \
					   utils/headers/crypto.h 

utils/source/fio.o: utils/headers/fio.h

utils/source/input.o: utils/headers/console.h \
					  utils/headers/input.h 

utils/source/path.o: utils/headers/path.h 

# Deleting bins and object files
clean:
	@echo "Cleaning up..."
	@sudo rm $(COMMANDS) $(UTILS) $(PWMAN) $(INIT)
	@sudo rm $(SYSTEM_PATH)/$(EXEC_NAME)
	@sudo rm $(SYSTEM_PATH)/$(INIT_NAME)
	@echo "Done"