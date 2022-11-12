#!/bin/bash

# This script builds pwman.c file

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test.o -g $ROOT_DIR/pwman.c \
    $ROOT_DIR/commands/source/* \
    $ROOT_DIR/utils/source/* \
    -lgpgme -lpthread -lX11