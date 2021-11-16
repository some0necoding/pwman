#include <stdio.h>
#include <sodium.h>
#include <termios.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define HASH_FILE_PATH "./.resources/.authashes"
#define HASH_FILE_SIZE crypto_pwhash_STRBYTES       // i.e 128 bytes

#define PASS_LENGTH 65                              // 64 bytes + '\0'
#define PASS_BUFF_LENGTH 64
#define HASH_LENGTH crypto_box_SEEDBYTES

#define MIN_LENGTH 15
#define MIN_NUMS 1
#define MIN_UPPER 1
#define MIN_SYMS 1
#define NUMS "0123456789"
#define UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

int signin(void);
int login(void);
int get_file_size(char *file_path);
int psm_read_pass(char *buff);
int auth_pass(char *password);
int store_key(char *bytes, char *file_path);
int get_hash(char *file_path, char *hash_and_salt);
int if_char_occur_one(char *str, char *str_of_char);
struct termios disable_terminal_echo();
void enable_terminal_echo(struct termios old);

/*-----------FUNCTIONS-DEFINITION-END-----------*/



/*-----------------FIXES-START------------------*/

// 1. all methods that return a pointer by passing it as an arguments should be modified 
//    to return the pointer as a return value.

/*------------------FIXES-END-------------------*/



// auth function returns 0 for good auth (no errors), 
// and -1 for errors.  

int auth(void) 
{
    int hash_file_size = get_file_size(HASH_FILE_PATH);
    int correct_file_size = HASH_FILE_SIZE;
    int auth_code = 0;

    if (hash_file_size == -1) {
        return -1;
    }

    if (hash_file_size != correct_file_size) {
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

int get_file_size(char *file_path)
{
    FILE *file = fopen(file_path, "r");
    int size = 0;
    
    if (!file) {
        perror("psm: allocation error\n");
        return -1;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        return -1;
    }

    size = ftell(file);

    return size;
}

// signin function returns 1 for true (good signin),
// 0 for false (bad signin == bad pswrd) and -1 for errors

int signin() 
{
    int buff_len = PASS_LENGTH;
    int hash_len = HASH_LENGTH;
    int pass_code = 0;
    int pass_len = 0;

    unsigned char *pass = (unsigned char *) sodium_malloc(buff_len);
    unsigned char *hash = (unsigned char *) sodium_malloc(hash_len);

    printf("%s", "choose a password: ");

    if ((pass_code = psm_read_pass(pass)) == -1) {
        return -1;
    } else if (pass_code == 0) {
        return 0;
    }
    
    printf("%s", "\n");

    if (auth_pass(pass) == -1) {
        return 0;
    }

    pass_len = strlen(pass);

    if (generate_masterkey(pass, hash) != 0) {
        return -1;
    }

    if (store_key(hash, HASH_FILE_PATH) == -1) {
        return -1;
    }

    sodium_free(pass);
    sodium_free(hash);

    return 1;
}

int psm_read_pass(char *buffer) 
{
    int bufsize = PASS_BUFF_LENGTH;
    int position = 0;
    char c = 0x00;
    struct termios old;

    if (!buffer) {
        perror("psm: allocation error\n");
        return -1;
    }

    old = disable_terminal_echo();

    while ((c = getchar()) != EOF && c != '\n') {
        buffer[position] = c;
        position++;
        if (position > bufsize) {
            enable_terminal_echo(old);
            printf("psm: password length cannot be larger than %d\n", bufsize);
            return 0;
        }
    }

    buffer[position] = '\0';

    enable_terminal_echo(old);

    return 1;
}

struct termios disable_terminal_echo() {

    struct termios old_t;
    struct termios new_t;

    (void) tcgetattr(fileno(stdin), &old_t);

    new_t = old_t;
    new_t.c_lflag &= ~ECHO;

    (void) tcsetattr(fileno(stdin), TCSAFLUSH, &new_t);

    return old_t;
}

void enable_terminal_echo(struct termios old_t) {
    (void) tcsetattr(fileno(stdin), TCSAFLUSH, &old_t);
}

int auth_pass(char *password) 
{
    int min_len = MIN_LENGTH;
    char *nums = NUMS;
    char *upper = UPPER;

    if (strlen(password) < min_len) {
        printf("%s\n", "Your password must have at least 15 characters long");
        return -1;
    } else if (if_char_occur_one(password, nums) == -1 || if_char_occur_one(password, upper) == -1) {
        printf("%s\n", "Your password must contain at least 1 number and 1 uppercase letter");
        return -1;
    }
    
    return 0;
}

int if_char_occur_one(char *str, char *str_of_char)
{
    int str_len = strlen(str_of_char);

    for (int i=0; i<str_len; i++) {
        if (strchr(str, str_of_char[i]) != NULL) {
            return 0;
        }
    }

    return -1;
}

int generate_masterkey(char *password, unsigned char *key)
{
    unsigned char *salt = (unsigned char *) sodium_malloc(crypto_pwhash_SALTBYTES);

    randombytes_buf(salt, sizeof salt);

    if (crypto_pwhash(key, sizeof key, 
                      password, strlen(password), 
                      salt, 
                      crypto_pwhash_OPSLIMIT_SENSITIVE, 
                      crypto_pwhash_MEMLIMIT_SENSITIVE, 
                      crypto_pwhash_ALG_DEFAULT) != 0) 
    {
        sodium_free(salt);
        return -1;
    }

    sodium_free(salt);
    return 0;
}

int store_key(unsigned char *key, char *file_path)
{
    if (write_key(key, file_path) != 0) {
        return -1;
    }

    if (encrypt(key, file_path) != 0) {
        return -1;
    }

    return 0;
}

int write_key(unsigned char *key, char *file_path)
{
    FILE *key_file = fopen(file_path, "w"); // 'w' opening will automatically clear the file
    int key_len = HASH_LENGTH;

    if (!key_file) {
        perror("psm: file opening error\n");
        return -1;
    }

    for (int i=0; i<key_len; i++) {
        int status = putc((int) key[i], key_file);
        if (status == EOF) {
            perror("psm: file writing error\n");
            return -1;
        }
    }

    fclose(key_file);

    return 0;
}

// login function will return 1 for true (access allowed),
// 0 for false (access denied) and -1 for errors.

int login()
{
    int buff_len = PASS_LENGTH;
    int hash_len = HASH_LENGTH;
    int input_pass_len = 0;
    int pass_reading_code = 0;

    char *input_pass = (char *) sodium_malloc(buff_len);
    char *stored_hash = (char *) sodium_malloc(hash_len);
    char *hash_file_path = HASH_FILE_PATH;
    
    if (!input_pass || !stored_hash) {
        perror("psm: allocation error\n");
        return -1;
    }

    printf("%s", "insert password: ");

    if ((pass_reading_code = psm_read_pass(input_pass)) == -1) {
        return -1;
    } else if (pass_reading_code == 0) {
        return 0;
    }

    printf("%s", "\n");

    input_pass_len = strlen(input_pass);

    if (get_hash(hash_file_path, stored_hash) == -1) {
        return -1;
    } 

    if (crypto_pwhash_str_verify(stored_hash, input_pass, input_pass_len) != 0) {
        printf("%s\n", "Wrong password!");
        return 0;
    }

    sodium_free(input_pass);
    sodium_free(stored_hash);

    return 1;
} 

int get_hash(char *file_path, char *hash) 
{
    int position = 0;
    char c = 0x00;

    FILE *hash_file = fopen(file_path, "r");

    if (!hash_file || !hash) {
        perror("psm: allocation error\n");
        return -1;
    }

    while ((c = getc(hash_file)) != EOF) {
        hash[position] = c;
        position++;
    }
    
    if (ferror(hash_file)) {
        perror("psm: file reading error\n");
        return -1;
    }

    fclose(hash_file);

    return 0;    
}