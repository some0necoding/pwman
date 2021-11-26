#ifndef HASHING
#define HASHING

#include <string.h>

// hashes a password "pass" of "pass_len" length
extern char *pass_hash(char *pass, size_t pass_len);

// stores a hash "hash" in a file stored at "file_path" path
extern int store_hash(char *hash, char *file_path);

// retrieves a hash "hash" in a file stored at "file_path" path
extern char *get_hash(char *file_path);

#endif