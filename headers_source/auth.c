#include "../headers/auth.h"

#include "../headers/sodiumplusplus.h"
#include "../headers/termiosplusplus.h"
#include "../headers/stdioplusplus.h"
#include "../headers/hashing.h"
#include "../headers/cryptography.h"
#include "../headers/input_acquisition.h"

#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define HASH_FILE_SIZE crypto_pwhash_STRBYTES       // i.e 128 bytes
#define HASH_FILE_PATH "./config_files/login.hash"
#define SALT_FILE_PATH "./config_files/crypto.salt"
#define ACCT_FILE_PATH "./config_files/accounts.list"
#define PASS_FILE_PATH "./config_files/passwords.list"

#define PASS_LENGTH 65                              // 64 bytes + '\0'
#define HASH_LENGTH crypto_box_SEEDBYTES

#define MIN_LENGTH 15
#define MAX_LENGTH 64
#define NUMS "0123456789"
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*------GLOBAL-VARIABLES-DEFINITION-START-------*/

unsigned char **subkeys;

/*-------GLOBAL-VARIABLES-DEFINITION-END--------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

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
    size_t mkey_len = crypto_kdf_KEYBYTES;
    size_t skey_len = crypto_kdf_BYTES_MAX;
    size_t salt_len = crypto_pwhash_SALTBYTES;

    char *pass = (char *) sodium_malloc(buff_len);
    char *hash = (char *) sodium_malloc(hash_len);
    char *hash_file_path = HASH_FILE_PATH;
    char *salt_file_path = SALT_FILE_PATH;
    char *acct_file_path = ACCT_FILE_PATH;
    char *pass_file_path = PASS_FILE_PATH;

    unsigned char *mkey = (unsigned char *) sodium_malloc(mkey_len);
    unsigned char *salt = (unsigned char *) sodium_malloc(salt_len);

    int skey_qty = 2;
    int skey_one = 0;
    int skey_two = 1;
    int ret_code = -1;

    if (!pass || !hash) {
        perror("psm: allocation error\n");
        return -1;
    }

    // getting the input
    printf("choose a password: ");

    if (!(pass = read_line_s())) {
        perror("psm: I/O error");
        goto ret;
    }
    
    printf("\n");

    // verifing the input
    if (auth_pass(pass) == -1) {
        ret_code = 0;
        goto ret;
    }

    // here starts the part where hash gets created and stored

    ret_code = -1;

    pass_len = strlen(pass);

    if (!(hash = pass_hash(pass, pass_len))) {
        perror("psm: cryptography error");
        goto ret;
    }

    if (store_hash(hash, hash_file_path) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    // here starts the part where two encryption keys that will be used to 
    // encrypt accounts and passwords are created out of the password.

    if (!(mkey = generate_masterkey(pass, &salt))) {
        perror("psm: cryptography error");
        goto ret;
    }

    if (write_salt(salt, salt_file_path) != 0) {
        perror("psm: cryptography error");
        goto ret;
    }

    // subkeys is a global variable that gets externed in order to use it in 
    // passwordmanager.c file. It is an array containing the two encryption keys.
    subkeys = (unsigned char **) sodium_malloc(sizeof(char) * skey_len);

    if (!(subkeys = generate_subkeys(skey_qty, mkey))) {
        perror("psm: cryptography error");
        goto ret;
    }

    if ((encrypt_file(acct_file_path, subkeys[skey_one]) != 0) | 
        (encrypt_file(pass_file_path, subkeys[skey_two]) != 0)) 
    {
        perror("psm: cryptography error");
        goto ret;
    }
    
    ret_code = 1;

ret:
    sodium_free(pass);
    sodium_free(hash);
    sodium_free(mkey);
    sodium_free(salt);
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
    int array_len = strlen(char_array);

    for (int i=0; i<array_len; i++) {
        if (strchr(str, char_array[i]) != NULL) {
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
    size_t mkey_len = crypto_kdf_KEYBYTES;
    size_t skey_len = crypto_kdf_BYTES_MAX;
    size_t salt_len = crypto_pwhash_SALTBYTES;

    char *pass = (char *) sodium_malloc(buff_len);
    char *hash = (char *) sodium_malloc(hash_len);
    char *hash_file_path = HASH_FILE_PATH;
    char *salt_file_path = SALT_FILE_PATH;

    unsigned char *mkey = (unsigned char *) sodium_malloc(mkey_len);
    unsigned char *salt = (unsigned char *) sodium_malloc(salt_len);

    int skey_qty = 2;
    int ret_code = -1;
    
    if (!pass || !hash) {
        perror("psm: allocation error\n");
        return -1;
    }

    // getting the input
    printf("insert password: ");

    if (!(pass = read_line_s())) {
        perror("psm: I/O error");
        goto ret;
    }

    printf("\n");

    // here starts the part were the hash gets generated and stored
    pass_len = strlen(pass);

    if (!(hash = get_hash(hash_file_path))) {
        perror("psm: I/O error");
        goto ret;
    } 

    if (crypto_pwhash_str_verify(hash, pass, pass_len) != 0) {
        printf("wrong password!\n");
        ret_code = 0;
        goto ret;
    }

    // here starts the part where encryption keys are generated

    ret_code = -1;

    if (get_salt(salt, salt_file_path) != 0) {
        perror("psm: cryptography error");
        goto ret;
    }

    if (!(mkey = generate_masterkey_with_salt(pass, salt))) {
        perror("psm: cryptography error");
        goto ret;
    }

    if (!(subkeys = generate_subkeys(skey_qty, mkey))) {
        perror("psm: cryptography error");
        goto ret;
    }

    ret_code = 1;

ret:
    sodium_free(pass);
    sodium_free(hash);
    sodium_free(mkey);
    sodium_free(salt);
    return ret_code;
}