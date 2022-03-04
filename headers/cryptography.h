#ifndef CRYPTOGRAPHY
#define CRYPTOGRAPHY

// encrypts a buffer "plain_buff" using a key "key", then it stores the result in the file located at "file_path" path.
int encrypt_buffer(unsigned char *plain_buff, unsigned char *key, char *file_path);

// encrypts a file stored at "file_path" using a key "key", then it stores the result in the same location as the input file.
int encrypt_file(char *file_path, unsigned char *key);

// decrypts a file stored at "file_path" using the key "key", then it stores the result in the same location as the input file.
unsigned char *decrypt_file(char *file_path, unsigned char *key);

// generates a masterkey out of a password "password" using an auto-generated random salt that gets stored in "salt" pointer.
unsigned char *generate_masterkey(char *password, unsigned char *salt);

// generates a masterkey out of a password "password" and a user-determined salt "salt" that must be of crypto_pwhash_SALTBYTES length.
unsigned char *generate_masterkey_with_salt(char *password, unsigned char *salt);

// generates qty subkeys out of an high entropy masterkey that can be generated or derived from a password
unsigned char **generate_subkeys(int qty, unsigned char *masterkey);

// writes a salt "salt" in a file stored at "file_path" path.
int write_salt(unsigned char *salt, char *file_path);

// retrieves a salt "salt" from a file stored at "file_path" path.
int get_salt(unsigned char *salt, char* file_path);

#endif