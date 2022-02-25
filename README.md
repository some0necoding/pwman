<!-- maybe here should be put a logo -->

<h1 align="center">passman</h1>

<!-- maybe here should be put a logo -->

<!-- links and badges here -->

## Basic Overview

Passman is a security-focused password manager written in C I made to get in touch with  a bit of
software security and learn more about this topic.  
Its simpler purpose is to store user's accounts along with passwords in encrypted format, in order
to improve user's security in the web. The use of a password manager makes it possible for the user
to adopt a different password for every account he has, so that if one account gets exploited the
others remain safe.

## Technologies

#### Cryptography

The algorithm used to support file cryptography in passman is XChaCha20Poly1305, that is an AEAD
(Authenticated Encryption with Additional Data) algorithm. AEAD algorithms are the best solution
to provide confidentiality, integrity and authenticity about the encrypted message.  
The algorithm is composed by two cryptographic primitives: XChaCha20 and Poly1305.  
The first one is an algorithm based on Salsa20, an AE (Authenticates) 

#### Storage

- XChaCha20Poly1305 (AEAD) algorithm

## Installation

#### Needed Libs

#### Use of make utility

```
task:                           command:
install passman                 make install
run passman                     passman
uninstall passman               make clean
```

## How to use passman

## Lincense