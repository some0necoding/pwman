#ifndef CRYPTOGRAPHY
#define CRYPTOGRAPHY

// encrypts a buffer "plain_buff" using a key "key", then it stores the result in the file located at "file_path" path.
int encrypt_buffer(unsigned char *plain_buff, unsigned char *key, char *file_path);

// encrypts a file stored at "file_path" using a key "key", then it stores the result in the same location as the input file.
int encrypt_file(char *file_path, unsigned char *key);

// decrypts a file stored at "file_path" using the key "key", then it stores the result in the same location as the input file.
unsigned char *decrypt_file(char *file_path, unsigned char *key);

// generates a masterkey out of a password "password" that gets stored in "key" buffer. 
int generate_masterkey(char *password, unsigned char *key);

// writes a masterkey "key" of "key_len" length in a file stored at "file_path" path.
int write_key(unsigned char *key, size_t key_len, char *file_path);

#endif