#!/usr/bin/bash

cd config_files
rm accounts.list && touch accounts.list && rm crypto.salt && touch crypto.salt && rm login.hash && touch login.hash && rm passwords.list && touch passwords.list
cd ..
gcc -o passman passwordmanager.c headers_source/* -lsodium
./passman