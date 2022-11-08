#!/bin/bash

# This script builds test_psm_rm.c test

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test.o $TEST_DIR/source/test_psm_rm.c \
    $ROOT_DIR/commands/source/psm_rm.c \
    $ROOT_DIR/utils/source/config.c \
    $ROOT_DIR/utils/source/fio.c