#!/bin/bash

PROJECT_ROOT="$HOME/coding/c/projects/pwman"
CURRENT_DIR=$PWD

cd $PROJECT_ROOT

printf "%s\n" "Compiling..."
gcc tests/dynstr/test.c src/utils/dynstr.c src/utils/estring.c -o tests/dynstr/test
printf "%s\n" "Done"

tests/dynstr/test

cd $CURRENT_DIR
