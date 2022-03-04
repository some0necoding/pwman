<h1 align="center">
    <img width="300" alt="PWM4N" src="https://github.com/some0necoding/pwman/blob/main/.github/pwman_logo_%232.svg">
</h1>

<!--h1 align="center">pwman</h1-->

<!-- links and badges here -->

## Basic Overview

Pwman (pronounced *pi-double-u-man*) is a security-focused password manager coded in C.
Its simpler purpose is to store user's accounts along with passwords in encrypted format, in order
to improve user's security in the web. The use of a password manager makes it possible for the user
to adopt a different password for every account he has, so that if one account gets exploited the
others remain safe.

## Installation

### Needed Libs

The only third party library needed to make Pwman run is [libsodium](https://doc.libsodium.org/installation),
which provides functions for file cryptography, user authentication and secure memory allocation.

### Installation using make

```
$ make install                      // install pwman
$ pwman                             // run pwman
$ make clean                        // uninstall pwman
```

## Cryptography

To encrypt user's data Pwman uses XChaCha20Poly1305 algorithm, that offers AEAD encryption.

<!--
The algorithm used to support file cryptography in pwman is XChaCha20Poly1305, an AEAD 
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
tag must match the one obtained by passing in the function the same key and the decrypted cyphertext. --> 


## Lincense

This project is licensed under the terms of the [**GPL**](https://github.com/some0necoding/pwman/blob/main/LICENSE.md) license.