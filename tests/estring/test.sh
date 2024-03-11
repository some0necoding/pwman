#!/bin/bash

PROJECT_ROOT="$HOME/coding/c/projects/pwman"
CURRENT_DIR=$PWD

cd $PROJECT_ROOT

printf "%s\n" "Compiling..."
gcc tests/estring/test.c src/utils/estring.c -o tests/estring/test
printf "%s\n" "Done"

tests/estring/test

cd $CURRENT_DIR
