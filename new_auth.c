#ifndef NULL
    #include <string.h>
#endif

#ifndef TERMIOS_PLUS_PLUS
    #include <termiosplusplus.h>
#endif

#ifndef STDIO_PLUS_PLUS
    #include <stdioplusplus.h>
#endif

#ifndef SODIUM_PLUS_PLUS
    #include <sodiumplusplus.h>
#endif

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

int auth(void);
int signin(void);
int login(void);
int psm_read_pass(char *buff);
int auth_pass(char *password);
int store_key(char *bytes, char *file_path);
int get_hash(char *file_path, char *hash_and_salt);
int if_char_occur_one(char *str, char *str_of_char);
int write_key(unsigned char *key, char *file_path);

/*-----------FUNCTIONS-DEFINITION-END-----------*/



/*-----------------FIXES-START------------------*/

// 1. all methods that return a pointer by passing it as an arguments should be modified 
//    to return the pointer as a return value.

/*------------------FIXES-END-------------------*/



// auth function returns 0 for good auth (no errors), 
// and -1 for errors.  

int auth(void) 
{
    int hash_file_size = fsize(HASH_FILE_PATH);
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

int auth_pass(char *password) 
{
    int min_len = MIN_LENGTH;
    char *nums = NUMS;
    char *upper = UPPER;

    if (strlen(password) < min_len) {
        printf("%s\n", "Your password must be at least 15 characters long");
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
        if (!strchr(str, str_of_char[i])) {
            return 0;
        }
    }

    return -1;
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