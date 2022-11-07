#!/bin/bash

TEST_DIR="$HOME/coding/c/projects/pwman/test"

if [ -z $1 ]; then
    echo "You forgot an argument"
    echo "Syntax: ./check_leaks.sh [COMMAND_SOURCE]"
else 
    ./build_test.sh $1

    valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    $TEST_DIR/bin/test.o > $TEST_DIR/debug/valgrind_output.txt
fi