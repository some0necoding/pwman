#ifndef CRYPTOGRAPHY
#define CRYPTOGRAPHY

// encrypts plain using openpgp public key of fingerprint fpr
const char *gpg_encrypt(char *plain, const char *fpr);

// decrypts cypher using openpgp secret key of fingerprint fpr
const char *gpg_decrypt(char *cypher, const char *fpr);

#endif