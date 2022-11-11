#!/bin/bash

# This script builds test_crypto.c test

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test.o -g $TEST_DIR/source/test_crypto.c \
    $ROOT_DIR/utils/source/crypto.c \
    $ROOT_DIR/utils/source/input.c \
    $ROOT_DIR/utils/source/console.c \
    -lgpgme