<h1 align="center">
    <img width="500" alt="PWM4N" src="https://github.com/some0necoding/pwman/blob/main/.github/pwman_logo.png">
</h1>

<!-- links and badges here -->

## Overview

Pwman (pronounced *pi-double-u-man*) is a command line password manager written in C. Its simpler purpose is 
to store user's accounts and passwords in encrypted format, in order to improve user's security. To do that it 
uses gpg asymmetric encryption.

## Installation

```
$ autoconf --install
$ ./configure 
$ make && sudo make install
```

Note that [gpgme](https://gnupg.org/software/gpgme/index.html) and [libsodium](https://doc.libsodium.org/) need to be installed on your system for pwman to run.

## Lincense

This project is licensed under the terms of the [**GPL**](https://github.com/some0necoding/pwman/blob/main/LICENSE.md) license.
