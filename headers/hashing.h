#ifndef HASHING
#define HASHING

// hashes a password "pass" of "pass_len" length
char *pass_hash(char *pass, size_t pass_len);

// stores a hash "hash" in a file stored at "file_path" path
int store_hash(char *hash, char *file_path);

// retrieves a hash "hash" in a file stored at "file_path" path
char *get_hash(char *file_path);

#endif