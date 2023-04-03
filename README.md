<h1 align="center">
    <img width="500" alt="PWM4N" src="https://github.com/some0necoding/pwman/blob/main/.github/pwman_logo.png">
</h1>

<!-- links and badges here -->

# TODO
 - pwman-init.c: check that X11 is running, create password store directory, 
				 choose gpg key and put everything in a config file. The config 
				 file should be placed under $HOME/.config or $HOME/.{appname} 
				 if the former does not exist.

 All of the above can be condesed into pwman.c

## Overview

Pwman (pronounced *pi-double-u-man*) is a command line password manager written in C. Its simpler purpose is 
to store user's accounts and passwords in encrypted format, in order to improve user's security. To do that it 
uses gpg asymmetric encryption.

## Installation

```
$ autoconf --install
$ ./configure 
$ make && sudo make install
$ ./pwman-init                // only on first run
$ ./pwman
```

Note that [gpgme](https://gnupg.org/software/gpgme/index.html) needs to be installed on your system for pwman to run (it's usually installed by default).

## Lincense

This project is licensed under the terms of the [**GPL**](https://github.com/some0necoding/pwman/blob/main/LICENSE.md) license.
