#ifndef NULL
    #include <string.h>
#endif

#ifndef SODIUM_PLUS_PLUS
    #include "headers/sodiumplusplus.h"
#endif

#ifndef STDIO_PLUS_PLUS
    #include "headers/stdioplusplus.h"
#endif

#include "headers/hashing.h"

// this function hashes a password pass of pass_len length using 
// libsodium library (doc.libsodium.org)
char *pass_hash(char *pass, size_t pass_len) 
{
    size_t hash_len = crypto_pwhash_STRBYTES;          // hash buffer length (doc.libsodium.org)
    char *hash = (char *) sodium_malloc(hash_len);     // allocating hash buffer (doc.libsodium.org)

    // the actual hashing function (doc.libsodium.org). 
    // (I cast into unsigned long long only here because I refuse to use this data type when someone invented size_t for this purpose)
    if (crypto_pwhash_str(hash, pass, (unsigned long long) pass_len, crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
        perror("psm: hash failed");
        return NULL;
    }

    return hash;
}

// this function stores a hash in a file
int store_hash(char *hash, char *file_path)
{
    size_t wlen;                                        // wlen stays for "Wrote (bytes) LENgth"
    size_t hash_len = crypto_pwhash_STRBYTES;           // hash buffer length (doc.libsodium.org)

    FILE *file = fopen(file_path, "wb");

    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    // writing hash value. hash values can be shorter than hash_len, so it's used > sign
    // to validate the writing (doc.libsodium.org for more info)
    if ((wlen = fwrite(hash, 1, hash_len, file)) > hash_len) {
        perror("psm: I/O error");
        return -1;
    }

    fclose(file);
    return 0;
}

// this function retrieves a hash from a file and saves it in a secure-allocated buffer
char *get_hash(char *file_path)
{
    size_t rlen;                                        // rlen stays for "Read (bytes) LENgth"
    size_t hash_len = crypto_pwhash_STRBYTES;           // hash buffer length (doc.libsodium.org)
    char *hash = (char *) sodium_malloc(hash_len);      // allocating hash buffer (doc.libsodium.org)

    FILE *file = fopen(file_path, "rb");

    if (!file) {
        perror("psm: allocation error");
        return NULL;
    }

    // reading hash value. hash values can be shorter than hash_len, so it's used > sign
    // to validate the reading (doc.libsodium.org for more info)
    if ((rlen = fread(hash, 1, hash_len, file)) > hash_len) {
        perror("psm: I/O error");
        return NULL;
    }

    fclose(file);
    return hash;
}