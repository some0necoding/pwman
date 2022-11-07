#!/bin/bash

TEST_DIR="$HOME/coding/c/projects/pwman/test"

valgrind \
--leak-check=yes \
--track-origins=yes \
--log-file=$TEST_DIR/debug/valgrind.log \
$TEST_DIR/bin/test.o > $TEST_DIR/debug/valgrind_output.txt
#--show-leak-kinds=all \