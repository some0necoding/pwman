#!/bin/bash

ROOT_DIR="$HOME/coding/c/projects/pwman"
TEST_DIR="$HOME/coding/c/projects/pwman/test"

if [ -z $1 ]; then
    echo "You missed an argument"
    echo "Syntax: ./build_test.sh [TEST_FILE]"
else
    gcc -o $TEST_DIR/bin/test.o -g \
    $ROOT_DIR/utils/source/* \
    $ROOT_DIR/commands/source/$1 \
    $TEST_DIR/source/test_$1 \
    -lsodium -lX11 -lpthread
fi