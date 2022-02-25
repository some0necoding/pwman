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

### Cryptography

The algorithm used to support file cryptography in passman is XChaCha20Poly1305, an AEAD 
(Authenticated Encryption with Additional Data) algorithm.   
AEAD encryption aims to provide three security and privacy goals:  
- confidentiality  
- integrity  
- authenticity    

The algorithm is composed of two cryptographic primitives: XChaCha20 and Poly1305.

#### XChaCha20 - Confidentiality 

XChaCha20 is the most recent and secure evolution of ChaCha20, based on Salsa20, a symmetric stream 
cypher that performs ARX (add-rotate-XOR) operations on the given stream using 256-bit key and 192-bit 
nonce.   
It is usually preferred over AES for CPUs where AES acceleration is not supported due to its better
performance.

#### Poly1305 - Integrity and Authenticity

Poly1305 is a cryptographic MAC (Message Authentication Code) used to verify authenticity and data 
integrity of a message.  
It works similarly to a digital signature function, but using symmetric encryption: provided a key and 
an input string (in this case the plaintext), the function returns a unique hash value (*tag*) that gets 
appended at the end of the cyphertext. To verify integrity and authenticity of the message, the appended 
tag must match the one obtained by passing in the function the same key and the decrypted plaintext.  

### Storage

## Installation

### Needed Libs

### Use of make utility

```
task:                           command:
install passman                 make install
run passman                     passman
uninstall passman               make clean
```

## How to use passman

## Lincense