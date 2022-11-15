#!/bin/bash

# This script builds pwman.c file

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test_pwman.o -g $ROOT_DIR/pwman.c \
    $ROOT_DIR/commands/source/* \
    $ROOT_DIR/utils/source/* \
    -lgpgme -lpthread -lX11

gcc -o $TEST_DIR/bin/test_pwman_init.o -g $ROOT_DIR/pwman-init.c \
    $ROOT_DIR/utils/source/config.c \
    $ROOT_DIR/utils/source/console.c \
    $ROOT_DIR/utils/source/crypto.c \
    $ROOT_DIR/utils/source/fio.c \
    $ROOT_DIR/utils/source/input.c \
    -lgpgme