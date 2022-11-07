#ifndef CRYPTOGRAPHY
#define CRYPTOGRAPHY

// encrypts plain using gpg public key of fingerprint fpr
const char *gpg_encrypt(char *plain, const char *fpr);

#endif