#!/bin/bash

# This script builds test_psm_get.c test

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test.o -g $TEST_DIR/source/test_psm_get.c \
    $ROOT_DIR/commands/source/psm_get.c \
    $ROOT_DIR/utils/source/* \
    -lgpgme -lX11 -lpthread