#ifndef CRYPTOGRAPHY
#define CRYPTOGRAPHY

#include <gpgme.h>
#include <stddef.h>

/* Encrypts plain using openpgp public key of fingerprint fpr */
const char *gpg_encrypt(const char *plain, const char *fpr);

/* Decrypts cypher using openpgp secret key of fingerprint fpr */
int gpg_decrypt(const char *cypher, const char *fpr, char **buf, size_t bufsize);

/* Returns an array containing local gpg keys */
gpgme_key_t *gpg_get_keys();

#endif
