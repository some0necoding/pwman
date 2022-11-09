#ifndef CRYPTOGRAPHY
#define CRYPTOGRAPHY

#include <stddef.h>

/* encrypts plain using openpgp public key of fingerprint fpr */
const char *gpg_encrypt(char *plain, const char *fpr);

/* Decrypts cypher using openpgp secret key of fingerprint fpr */
int gpg_decrypt(char *cypher, const char *fpr, char **buf, size_t bufsize);

#endif