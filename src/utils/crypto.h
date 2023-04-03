#ifndef CRYPTO_UTIL
#define CRYPTO_UTIL

#include <gpgme.h>
#include <stddef.h>

/* Encrypts plain using openpgp public key of fingerprint fpr */
const char *gpg_encrypt(const char *plain, const char *fpr);

/* Decrypts file using openpgp secret key of fingerprint fpr */
int gpg_decrypt_file(const char *filename, const char *fpr, char **buf, size_t bufsize);

/* Returns an array containing local gpg keys */
gpgme_key_t *gpg_get_keys();

#endif
