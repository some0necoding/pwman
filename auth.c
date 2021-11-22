#ifndef NULL
    #include <string.h>
#endif

#ifndef TERMIOS_PLUS_PLUS
    #include "termiosplusplus.h"
#endif

#ifndef SODIUM_PLUS_PLUS
    #include "sodiumplusplus.h"
#endif

#ifndef STDIO_PLUS_PLUS
    #include "stdioplusplus.h"
#endif

#include "hashing.h"
#include "cryptography.h"
#include "input_acquisition.h"

/*----------CONSTANTS-DEFINITION-START----------*/

#define HASH_FILE_PATH "./config_files/login.hash"
#define HASH_FILE_SIZE crypto_pwhash_STRBYTES       // i.e 128 bytes

#define PASS_LENGTH 65                              // 64 bytes + '\0'
#define HASH_LENGTH crypto_box_SEEDBYTES

#define MIN_LENGTH 15
#define MAX_LENGTH 64
#define NUMS "0123456789"
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int auth(void);
int signin(void);
int login(void);
int auth_pass(char *password);
int if_char_occur_one(char *str, char *str_of_char);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

// auth function returns 0 for good auth (no errors), 
// and -1 for errors.  

int auth(void) 
{
    int file_size = fsize(HASH_FILE_PATH);
    int correct_file_size = HASH_FILE_SIZE;
    int auth_code = 0;

    if (file_size == -1) {
        return -1;
    }

    if (file_size != correct_file_size) {
        // delete the encrypted password file
        while ((auth_code = signin()) == 0);
    } else {
        while ((auth_code = login()) == 0);
    }

    if (auth_code == -1) {
        return -1;
    }

    return 0;
}

// signin function returns 1 for true (good signin),
// 0 for false (bad signin == bad pswrd) and -1 for errors

int signin() 
{
    size_t buff_len = PASS_LENGTH;
    size_t hash_len = HASH_LENGTH;
    size_t pass_len = 0;

    char *pass = (unsigned char *) sodium_malloc(buff_len);
    char *hash = (unsigned char *) sodium_malloc(hash_len);
    char *file_path = HASH_FILE_PATH;

    int ret_code = -1;

    if (!pass || !hash) {
        perror("psm: allocation error\n");
        return -1;
    }

    printf("choose a password: ");

    if (!(pass = read_line_s())) {
        perror("psm: I/O error");
        goto ret;
    }
    
    printf("\n");

    if (auth_pass(pass) == -1) {
        ret_code = 0;
        goto ret;
    }

    pass_len = strlen(pass);

    if (!(hash = pass_hash(pass, pass_len))) {
        perror("psm: cryptography error");
        ret_code = -1;
        goto ret;
    }

    if (store_hash(hash, file_path) != 0) {
        perror("psm: I/O error");
        ret_code = -1;
        goto ret;
    }

    // here I need to generate a masterkey

    // here I need to generate two subkeys from the masterkey
    
    ret_code = 1;

ret:
    sodium_free(pass);
    sodium_free(hash);
    return ret_code;
}

int auth_pass(char *password) 
{
    size_t min_len = MIN_LENGTH;
    size_t max_len = MAX_LENGTH;
    size_t str_len;

    char *nums = NUMS;
    char *upper = UPPER;

    str_len = strlen(password);

    if (str_len < min_len) {
        printf("your password must be at least 15 characters long\n");
        return -1;
    } else if (str_len > max_len) {
        printf("your password has to be maximum 64 characters long\n");
        return -1;
    } else if (if_char_occur_one(password, nums) == -1 || if_char_occur_one(password, upper) == -1) {
        printf("your password must contain at least 1 number and 1 uppercase letter\n");
        return -1;
    }
    
    return 0;
}

int if_char_occur_one(char *str, char *char_array)
{
    int str_len = strlen(char_array);

    for (int i=0; i<str_len; i++) {
        if (!strchr(str, char_array[i])) {
            return 0;
        }
    }

    return -1;
}

// login function will return 1 for true (access allowed),
// 0 for false (access denied) and -1 for errors.

int login()
{
    size_t buff_len = PASS_LENGTH;
    size_t hash_len = HASH_LENGTH;
    size_t pass_len = 0;

    char *pass = (char *) sodium_malloc(buff_len);
    char *hash = (char *) sodium_malloc(hash_len);
    char *file_path = HASH_FILE_PATH;

    int ret_code = -1;
    
    if (!pass || !hash) {
        perror("psm: allocation error\n");
        return -1;
    }

    printf("insert password: ");

    if (!(pass = read_line_s())) {
        perror("psm: I/O error");
        goto ret;
    }

    printf("\n");

    pass_len = strlen(pass);

    if (!(hash = get_hash(file_path))) {
        perror("psm: I/O error");
        goto ret;
    } 

    if (crypto_pwhash_str_verify(hash, pass, pass_len) != 0) {
        printf("wrong password!\n");
        ret_code = 0;
        goto ret;
    }

    ret_code = 1;

ret:
    sodium_free(pass);
    sodium_free(hash);
    return ret_code;
}