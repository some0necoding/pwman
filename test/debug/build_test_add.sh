#!/bin/bash

# This script builds test_psm_add.c test

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test.o -g $TEST_DIR/source/test_psm_add.c \
    $ROOT_DIR/commands/source/psm_add.c \
    $ROOT_DIR/utils/source/config.c \
    $ROOT_DIR/utils/source/console.c \
    $ROOT_DIR/utils/source/crypto.c \
    $ROOT_DIR/utils/source/fio.c \
    $ROOT_DIR/utils/source/input.c \
    -lgpgme