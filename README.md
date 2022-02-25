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

The algorithm used to support file cryptography in passman is XChaCha20Poly1305, an AEAD 
(Authenticated Encryption with Additional Data) algorithm. AEAD algorithms aim to provide three 
security and privacy goals:  
- confidentiality  
- integrity  
- authenticity  
   
The algorithm is composed by two cryptographic primitives: XChaCha20 and Poly1305.  
The first one is an based on Salsa20, a stream cypher that uses ARX (add-rotate-XOR) operations.
Poly1305 is a cryptographic MAC (message authentication code) used to verify authenticity and
integrity of the message. 

#### Storage

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