#ifndef STDIO_PLUS_PLUS
#define STDIO_PLUS_PLUS

#include <stddef.h>
#include <stdio.h>

// returns the size of a null-terminated file.
size_t fsize(char *file_path);

// securely stores all bytes comprised between the beginning of the file and EOF into
// "ret_buff" (that needs to be allocated using sodium_malloc).
int fgetalls(char *file_path, char **ret_buff, size_t ret_buff_size);

// stores all bytes comprised between the beginning of the file and EOF in a secure way 
// into "ret_buff".
int fgetall(char *file_path, char **ret_buff, size_t ret_buff_size);

// securely stores all bytes comprised between start_pos (inclusive) and EOF into 
// "ret_buff" (that needs to be allocated using sodium_malloc).
int fgetfroms(char *file_path, int start_pos, char **ret_buff, size_t ret_buff_size);

// securely stores all bytes comprised between start_pos (inclusive) and end_pos (exclusive) 
// into "ret_buff" (that needs to be allocated using sodium_malloc).
int fgetfromtos(char *file_path, int start_pos, int end_pos, char **ret_buff, size_t ret_buff_size);

// it reads a single line from the file stored at file_path
int freadline(FILE *file, char **ret_buff, size_t bufsize);

#endif