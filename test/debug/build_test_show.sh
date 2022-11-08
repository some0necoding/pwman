#!/bin/bash

# This script builds test_psm_show.c test

TEST_DIR="$HOME/coding/c/projects/pwman/test"
ROOT_DIR="$HOME/coding/c/projects/pwman"

gcc -o $TEST_DIR/bin/test.o $TEST_DIR/source/test_psm_show.c \
    $ROOT_DIR/commands/source/psm_show.c \
    $ROOT_DIR/utils/source/config.c \
    $ROOT_DIR/utils/source/fio.c