<h1 align="center">
    <img width="500" alt="PWM4N" src="https://github.com/some0necoding/pwman/blob/main/.github/pwman_logo.png">
</h1>

<!-- links and badges here -->

## Basic Overview

Pwman (pronounced *pi-double-u-man*) is a command line password manager written in C. Its simpler purpose is 
to store user's accounts and passwords in encrypted format, in order to improve user's security. The use of 
a password manager makes it possible for the user to adopt a different password for every account he has, so 
that if one account gets exploited the others remain safe.

## Features
- GPG encryption
- Plaintext handled only in RAM
- Secure input for passwords
- CLI == faster

## Installation

### Needed Libs

The only third party library needed to make Pwman run is [gpgme](https://gnupg.org/software/gpgme/index.html),
which provides functions for asymmetric cryptography.

### Installation using make

```
$ ./configure.sh
$ make install
$ pwman-init
$ pwman
```

## Lincense

This project is licensed under the terms of the [**GPL**](https://github.com/some0necoding/pwman/blob/main/LICENSE.md) license.
