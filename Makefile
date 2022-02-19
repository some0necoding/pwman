# this file simply compiles all source code

CC = gcc

FLAG_1 = -lsodium
FLAG_2 = -lX11
FLAG_3 = -lpthread

MAIN_SRC = passwordmanager.c
HEAD_SRC = headers_source/*.c

EXEC_NAME = passman

compile:
	@sudo $(CC) -o /usr/local/bin/$(EXEC_NAME) $(MAIN_SRC) $(HEAD_SRC) $(FLAG_1) $(FLAG_2) $(FLAG_3)