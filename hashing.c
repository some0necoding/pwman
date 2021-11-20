#ifndef NULL
    #include <string.h>
#endif

#ifndef STDIO_PLUS_PLUS
    #include <stdioplusplus.h>

#ifndef SODIUM_PLUS_PLUS
    #include <sodiumplusplus.h>
#endif

/*----------CONSTANTS-DEFINITION-START----------*/

/*-----------CONSTANTS-DEFINITION-END-----------*/



/*----------FUNCTIONS-DEFINITION-START----------*/

char *pass_hash(char *pass, size_t pass_len);
int store_hash(char *hash, char *file_path);
int get_hash(char *hash, char *file_path);
/*-----------FUNCTIONS-DEFINITION-END-----------*/

char *pass_hash(char *pass, size_t pass_len) 
{
    size_t hash_len = crypto_pwhash_STRBYTES;
    char *hash = (char *) sodium_malloc(hash_len);

    if (crypto_pwhash_str(hash, pass, (unsigned long long) pass_len, crypto_pwhash_OPSLIMIT_SENSITIVE, crypto_pwhash_MEMLIMIT_SENSITIVE) != 0) {
        perror("psm: hash failed");
        return NULL;
    }

    return hash;
}

int store_hash(char *hash, char *file_path)
{
    size_t wlen;
    size_t hash_len = crypto_pwhash_STRBYTES;

    FILE *file = fopen(file_path, "wb");

    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    if ((wlen = fwrite(hash, 1, hash_len, file)) != hash_len) {
        perror("psm: I/O error");
        return -1;
    }

    fclose(file);
    return 0;
}

char *get_hash(char *file_path)
{
    size_t rlen;
    size_t hash_len = crypto_pwhash_STRBYTES;
    char *hash = (char *) sodium_malloc(hash_len);

    FILE *file = fopen(file_path, "rb");

    if (!file) {
        perror("psm: allocation error");
        return NULL;
    }

    if ((rlen = fread(hash, 1, hash_len, file)) != 0) {
        perror("psm: I/O error");
        return NULL;
    }

    fclose(file);
    return hash;
}