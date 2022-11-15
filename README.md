<h1 align="center">
    <img width="500" alt="PWM4N" src="https://github.com/some0necoding/pwman/blob/main/.github/pwman_logo.png">
</h1>

<!-- links and badges here -->

## Basic Overview

Pwman (pronounced *pi-double-u-man*) is a command line password manager written in C. Its simpler purpose is 
to store user's accounts and passwords in encrypted format, in order to improve user's security. To do that it 
uses gpg asymmetric encryption.

## Installation

### Needed Libs


[gpgme](https://gnupg.org/software/gpgme/index.html), which provides functions for gpg encryption, is needed 
to make Pwman run. Even if it's likely already installed on your system, check for its installation.

### Installation using make

```
$ ./configure.sh
$ make install
$ pwman-init                // only on first run
$ pwman
```

## Lincense

This project is licensed under the terms of the [**GPL**](https://github.com/some0necoding/pwman/blob/main/LICENSE.md) license.
