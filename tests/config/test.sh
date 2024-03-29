#!/bin/bash

PROJECT_ROOT="$HOME/coding/c/projects/pwman"
CURRENT_DIR=$PWD

cd $PROJECT_ROOT

printf "%s\n" "Compiling..."
gcc tests/config/test.c src/utils/dynstr.c src/utils/estring.c -lpcre -o tests/config/test
printf "%s\n" "Done"

tests/config/test
rm tests/config/test

cd $CURRENT_DIR
