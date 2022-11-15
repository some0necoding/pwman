#!/bin/bash

if [[ `echo $XDG_SESSION_TYPE` -ne "x11" ]]; then
    echo "You need to have X11 running for pwman to work"
else
    echo "Everything is good!"
    echo "Type \"make install\" to install pwman"
fi