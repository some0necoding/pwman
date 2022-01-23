#include "../headers/cryptography.h"

#include "../headers/sodiumplusplus.h"
#include "../headers/stdioplusplus.h"

#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define CHUNK_SIZE 4096
#define CONTEXT "decrypt_"

/*-----------CONSTANTS-DEFINITION-END-----------*/



/*----------FUNCTIONS-DEFINITION-START----------*/

unsigned char *encrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t *cypher_text_len);
unsigned char *decrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff);
int write_cypher_to_file(unsigned char *header, size_t header_len, unsigned char *cypher_text, size_t cypher_text_len, char *file_path);

/*-----------FUNCTIONS-DEFINITION-END-----------*/



/*---------------ENCRYPTION-START---------------*/

// It encrypts the buffer "plain_buff" using the key "key" and stores it in the file 
// located at "file_path". 
// The key point here is that all the encryption process is done in memory using buffers, 
// so that is only used a single file and not an encrypted file and a decrypted file.
// Having a plain text file stored in the filesystem is not a good thing. It'd be better 
// to handle plain text only in memory for security purposes:
//      - encryption: plain buffer -> encrypted file; 
//      - decryption: encrypted file -> plain buffer.
// A lot of forensic procedures exist to retrieve deleted or hidden files from disks, 
// instead memory is easily cleanable.
int encrypt_buffer(unsigned char *plain_buff, unsigned char *key, char *file_path)
{
    size_t cypher_text_len = 0;
    size_t header_len = crypto_secretstream_xchacha20poly1305_HEADERBYTES;
    unsigned char *cypher_text;
    unsigned char *header = (unsigned char *) sodium_malloc(header_len);
    crypto_secretstream_xchacha20poly1305_state state;
    int ret_code = -1;

    // check for good allocation
    if (!header) {
        perror("psm: allocation error");
        goto ret;
    }

    // initialize a state with a key and stores the output in the header
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        perror("psm: header init failed");
        goto ret;
    }

    // encrypt the buffer
    if (!(cypher_text = encrypt(&state, plain_buff, &cypher_text_len))) {
        goto ret;
    }
    
    // write the encrypted buff into the file
    if (write_cypher_to_file(header, header_len, cypher_text, cypher_text_len, file_path) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(header);
    return ret_code;
}

// To make the header more complete there's also an encryption function decrypted file -> encrypted file.
// It can be useful in cases were the machine and storage are not easily vulnerable (i.e. servers, even
// if idk who the hell uses C scripts inside a server).
int encrypt_file(char *file_path, unsigned char *key)
{
    size_t cypher_text_len = 0;
    size_t header_len = crypto_secretstream_xchacha20poly1305_HEADERBYTES;
    unsigned char *plain_text;
    unsigned char *cypher_text;
    unsigned char *header = (unsigned char *) sodium_malloc(header_len);
    crypto_secretstream_xchacha20poly1305_state state;
    int ret_code = -1;

    // check for good allocation
    if (!header) {
        perror("psm: allocation error");
        goto ret;
    }

    // initialize a state with a key and stores the output in the header
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        perror("psm: header init failed");
        goto ret;
    }

    // get the entire file content
    if (!(plain_text = fgetalls(file_path))) {
        perror("psm: I/O error");
        goto ret;
    }

    // encrypt the file content
    if (!(cypher_text = encrypt(&state, plain_text, &cypher_text_len))) {
        goto ret;
    }
    
    // write the encrypted buff into the file
    if (write_cypher_to_file(header, header_len, cypher_text, cypher_text_len, file_path) != 0) {
        perror("psm: I/O error");
        return -1;
    }

    ret_code = 0;

ret:
    sodium_free(header);
    return ret_code;
}

// this function encrypts a buffer using the libsodium library, in order to understand well the code you should be aware of what's inside 
// its documentation (doc.libsodium.org). 
// Basically this function will split up the buffer into smaller buffers of CHUNK_SIZE size, it will then encrypt them one by one 
// and lastly it will store them in a bigger buffer called ret_buff that will be returned.
unsigned char *encrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t *cypher_text_len)
{
    size_t old_buf_size;
    size_t dec_cnk_size = CHUNK_SIZE;
    size_t enc_cnk_size = CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES; // doc.libsodium.org for more info about this.

    unsigned char *cnk_buff = (unsigned char *) sodium_malloc(dec_cnk_size); // sub-buffer of the plaintext (doc.libsodium.org for more info).
    unsigned char *out_buff = (unsigned char *) sodium_malloc(enc_cnk_size); // this buffer contains the encrypted cnk_buff.
    unsigned char *ret_buff = (unsigned char *) sodium_malloc(enc_cnk_size); // buffer that contains all encrypted sub-buffers needed to cover the entire plaintext.
    unsigned char *ret_arr = NULL;

    int big_buff_pos = 0;
    int cnk_buff_pos = 0;
    int ret_buff_pos = 0;

    unsigned char c;
    unsigned long long out_len; // the length of an out_buff (doc.libsodium.org for more info).

    /*-----TEST-----*/

    char dec_ch;
    size_t dec_len = strlen(buff);

    printf("decrypted buff (%d chars)\n", dec_len);

    for (int dec_p=0; dec_p<dec_len; dec_p++) {   
        if (((dec_ch = buff[dec_p]) >= 48 && dec_ch <= 57) || (dec_ch >= 65 && dec_ch <= 90) || (dec_ch >= 97 && dec_ch <= 122)) { 
            printf("%c", dec_ch);
        } else {
            printf(" %d ", dec_ch);
        }
    }
    
    printf("\n");

    /*-----TEST-----*/

    while ((c = buff[big_buff_pos]) != '\0') {
        // if the cnk_buff is not full continue writing bytes into it.
        if (cnk_buff_pos < CHUNK_SIZE) {
            cnk_buff[cnk_buff_pos] = c;
            cnk_buff_pos++;
            big_buff_pos++;
        }
        // if the cnk_buff is full encrypt it and store it in ret_buff.
        else {

            // doc.libsodium.org for more info about this.
            if(crypto_secretstream_xchacha20poly1305_push(state, out_buff, &out_len, cnk_buff, cnk_buff_pos, NULL, 0, 0) != 0) {
                perror("psm: encryption: corrupted chunk");
                goto ret;
            }

            // copying out_buff into ret_buff.
            memcpy((void *) ret_buff+ret_buff_pos, (void *) out_buff, (size_t) out_len);
            // increasing the ret_buff's "cursor" (== index, it's an array) by the number of bytes contained in the sub-buffer just added.
            ret_buff_pos += (int) out_len;

            // if ret_buff is not large enough it gets stretched (notice that sodium_realloc is not present in sodium, but it's custom).
            if (ret_buff_pos >= enc_cnk_size) {
                old_buf_size = enc_cnk_size;
                enc_cnk_size += CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES;
                ret_buff = (unsigned char *) sodium_realloc(ret_buff, old_buf_size, enc_cnk_size);
            }

            // cursor position gets zeroed so that cnk_buff can be refilled from 0 to CHUNK_SIZE again (the buffer is basically emptied,
            // letting new data overwrite the old one).
            cnk_buff_pos = 0;
            // the byte that was hanging around since the cnk_buff was full is pushed in the "emptied" cnk_buff.
            cnk_buff[cnk_buff_pos] = c;
            cnk_buff_pos++;
            big_buff_pos++;
        }
    }

    if (crypto_secretstream_xchacha20poly1305_push(state, out_buff, &out_len, cnk_buff, cnk_buff_pos, NULL, 0, crypto_secretstream_xchacha20poly1305_TAG_FINAL) != 0) {
        perror("psm: encryption: corrupted chunk");
        goto ret;
    }

    memcpy((void *) ret_buff+ret_buff_pos, (void *) out_buff, (size_t) out_len);
    ret_buff_pos += (int) out_len;

    // assigning ret_buff size to a pointer so that it can be accessed outside this function
    *cypher_text_len = ret_buff_pos;

    // null-terminating ret_buff
    ret_buff[ret_buff_pos] = '\0';

    // copying ret_buff into ret_arr
    ret_arr = (unsigned char *) sodium_malloc(ret_buff_pos + 1);
    strncpy(ret_arr, ret_buff, ret_buff_pos);
    ret_arr[ret_buff_pos] = '\0';

    /*-----TEST-----*/

    char enc_ch;
    size_t enc_len = strlen(ret_arr);

    printf("encrypted buff (%d chars)\n", enc_len);

    for (int enc_p=0; enc_p<enc_len; enc_p++) {
        if (((enc_ch = ret_arr[enc_p]) >= 48 && enc_ch <= 57) || (enc_ch >= 65 && enc_ch <= 90) || (enc_ch >= 97 && enc_ch <= 122)) { 
            printf("%c", enc_ch);
        }  else {
            printf(" %d ", enc_ch);
        }
    }
    
    printf("\n");

    /*-----TEST-----*/

ret:
    // always free memory, mostly if secured, cuz we don't wanna have buffers hanging around mlocked.
    sodium_free(cnk_buff);
    sodium_free(out_buff);
    sodium_free(ret_buff);
    return ret_arr;
}

// the function simply writes some bytes from a buffer to a stream (header + cyhertext -> encrypted file), 
// but with some error checking (always do error checking).
int write_cypher_to_file(unsigned char *header, size_t header_len, unsigned char *cypher_text, size_t cypher_text_len, char *file_path)
{
    size_t rlen;
    FILE *file = fopen(file_path, "w");

    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    if ((rlen = fwrite(header, 1, header_len, file)) != header_len) {
        perror("psm: I/O error0");
        fclose(file);
        return -1;
    }

    if ((rlen = fwrite(cypher_text, 1, cypher_text_len, file)) != cypher_text_len) {
        perror("psm: I/O error1");
        fclose(file);
        return -1;
    }

    fclose(file);

    return 0;
}

/*----------------ENCRYPTION-END----------------*/



/*---------------DECRYPTION-START---------------*/

// It decrypts the file located at "file_path" using the key "key". 
// The key point here is that all the decryption process is done in memory using buffers, 
// so that is only used a single file and not an encrypted file and a decrypted file.
// Having a plain text file stored in the filesystem is not a good thing. It'd be better to handle 
// plain text only in memory for security purposes (encryption: plain buffer -> encrypted file; 
// decryption: encrypted file -> plain buffer).
// A lot of forensic procedures exist to retrieve deleted or hidden files from disks, 
// instead memory is easily cleanable.
unsigned char *decrypt_file(char *file_path, unsigned char *key)
{
    size_t rlen;
    size_t header_len = crypto_secretstream_xchacha20poly1305_HEADERBYTES;
    size_t plain_text_len = 0;

    unsigned char *plain_text;
    unsigned char *cypher_text;
    unsigned char *header = (unsigned char *) sodium_malloc(header_len);
    unsigned char *ret_buff = NULL;
    crypto_secretstream_xchacha20poly1305_state state;

    FILE *file = fopen(file_path, "rb");

    if (!file) {
        perror("psm: I/O error");
        return NULL;
    }

    if ((rlen = fread(header, 1, header_len, file)) != header_len) {
        perror("psm: I/O error");
        goto ret;
    }

    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        perror("psm: incomplete header");
        goto ret;
    }

    if (!(cypher_text = fgetfroms(file_path, header_len))) {
        perror("psm: I/O error");
        goto ret;
    }

    if (!(plain_text = decrypt(&state, cypher_text))) {
        goto ret;
    }

    plain_text_len = strlen(plain_text);
    ret_buff = (unsigned char *) sodium_malloc(plain_text_len + 1);
    strncpy(ret_buff, plain_text, plain_text_len);
    ret_buff[plain_text_len] = '\0';

ret:
    sodium_free(header);
    fclose(file);
    return ret_buff;
}

// this function decrypts a buffer using the libsodium library, in order to understand well the code you should be aware of what's inside 
// its documentation (doc.libsodium.org). 
// Basically this function will split up the buffer into smaller buffers of CHUNK_SIZE size, it will then decrypt them one by one 
// and lastly it will store them in a bigger buffer called ret_buff that will be returned.
unsigned char *decrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff)
{
    size_t old_buf_size;
    size_t dec_cnk_size = CHUNK_SIZE;
    size_t enc_cnk_size = CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES; // doc.libsodium.org for more info about this.

    unsigned char *cnk_buff = (unsigned char *) sodium_malloc(enc_cnk_size); // sub-buffer of the cyphertext (doc.libsodium.org for more info).
    unsigned char *out_buff = (unsigned char *) sodium_malloc(dec_cnk_size); // this buffer contains the decrypted cnk_buff.
    unsigned char *ret_buff = (unsigned char *) sodium_malloc(dec_cnk_size); // buffer that contains all decrypted sub-buffers needed to cover the entire cyphertext.
    unsigned char *ret_arr = NULL;

    unsigned char tag;

    int big_buff_pos = 0;
    int cnk_buff_pos = 0;
    int ret_buff_pos = 0;

    unsigned char c;
    unsigned long long out_len; // the length of an out_buff (doc.libsodium.org for more info).

    /*-----TEST-----*/

    char enc_ch;
    size_t enc_len = strlen(buff);

    printf("encrypted buff (%d chars)\n", enc_len);

    for (int enc_p=0; enc_p<enc_len; enc_p++) {
        if (((enc_ch = buff[enc_p]) >= 48 && enc_ch <= 57) || (enc_ch >= 65 && enc_ch <= 90) || (enc_ch >= 97 && enc_ch <= 122)) { 
            printf("%c", enc_ch);
        }  else {
            printf(" %d ", enc_ch);
        }
    }

    printf("\n");
    
    /*-----TEST-----*/

    while ((c = buff[big_buff_pos]) != '\0') {
        // if the cnk_buff is not full continue writing bytes into it.
        if (cnk_buff_pos < enc_cnk_size) {
            cnk_buff[cnk_buff_pos] = c;
            cnk_buff_pos++;
            big_buff_pos++;
        }
        // if the cnk_buff is full decrypt it and store it in ret_buff.
        else {

            // doc.libsodium.org for more info about this.
            if(crypto_secretstream_xchacha20poly1305_pull(state, out_buff, &out_len, &tag, cnk_buff, cnk_buff_pos, NULL, 0) != 0) {
                perror("psm: decryption: corrupted chunk");
                goto ret;
            }

            // checking for premature end (end of file reached before the end of the stream)
            if(tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL) {
                perror("psm: decryption: EOF reached before the end of the stream");
                goto ret;
            }

            // copying out_buff into ret_buff.
            memcpy((void *) ret_buff+ret_buff_pos, (void *) out_buff, (size_t) out_len);
            // increasing the ret_buff's "cursor" (== index, it's an array) by the number of bytes contained in the sub-buffer just added.
            ret_buff_pos += (int) out_len;

            // if ret_buff is not large enough it gets stretched (notice that sodium_realloc is not present in sodium but it's custom).
            if (ret_buff_pos >= dec_cnk_size) {
                old_buf_size = dec_cnk_size;
                dec_cnk_size += CHUNK_SIZE;
                ret_buff = (unsigned char *) sodium_realloc(ret_buff, old_buf_size, dec_cnk_size);
            }

            // cursor position gets zeroed so that cnk_buff can be refilled from 0 to CHUNK_SIZE again (the buffer is basically emptied,
            // letting new data overwrite the old one).
            cnk_buff_pos = 0;
            // the byte that was hanging around since cnk_buff was full is pushed in the "emptied" cnk_buff.
            cnk_buff[cnk_buff_pos] = c;
            cnk_buff_pos++;
            big_buff_pos++;
        }
    }

    if (crypto_secretstream_xchacha20poly1305_pull(state, out_buff, &out_len, &tag, cnk_buff, cnk_buff_pos, NULL, 0) != 0) {
        perror("psm: decryption: corrupted chunk");
        goto ret;
    }

    memcpy((void *) ret_buff+ret_buff_pos, (void *) out_buff, (size_t) out_len);
    ret_buff_pos += (int) out_len;

    // if ret_buff is full it gets stretched by 1 cuz we need to add a '\0' at the end
    if (ret_buff_pos >= dec_cnk_size) {
        old_buf_size = dec_cnk_size;
        dec_cnk_size += 1;
        ret_buff = (unsigned char *) sodium_realloc(ret_buff, old_buf_size, dec_cnk_size);
    }

    // null-terminating ret_buff
    ret_buff[ret_buff_pos] = '\0';

    // copying ret_buff into ret_arr
    ret_arr = (unsigned char *) sodium_malloc(ret_buff_pos + 1);
    strncpy(ret_arr, ret_buff, ret_buff_pos);
    ret_arr[ret_buff_pos] = '\0';

    /*-----TEST-----*/

    char dec_ch;
    size_t dec_len = strlen(ret_arr);

    printf("decrypted buff (%d chars)\n", dec_len);

    for (int dec_p=0; dec_p<dec_len; dec_p++) {
        if (((dec_ch = ret_arr[dec_p]) >= 48 && dec_ch <= 57) || (dec_ch >= 65 && dec_ch <= 90) || (dec_ch >= 97 && dec_ch <= 122)) { 
            printf("%c", dec_ch);
        }  else {
            printf(" %d ", dec_ch);
        }
    }

    printf("\n");
    
    /*-----TEST-----*/

ret:
    // always free memory, mostly if secured, cuz we don't wanna have buffers hanging around mlocked.
    sodium_free(cnk_buff);
    sodium_free(out_buff);
    sodium_free(ret_buff);
    return ret_arr;
}

/*----------------DECRYPTION-END----------------*/



/*--------------KEY-HANDLING-START--------------*/

// this function generates an high entropy masterkey out of a password and
// a user-provided salt
unsigned char *generate_masterkey_with_salt(char *password, unsigned char *salt)
{
    size_t key_len = crypto_kdf_KEYBYTES;
    unsigned char *key = (unsigned char *) sodium_malloc(key_len);

    if (!salt || !key) {
        perror("psm: allocation error");
        sodium_free(key);
        return NULL;
    }

    // this creates the actual hash using the password and the salt (doc.libsodium.org)
    if (crypto_pwhash(key, 
                      (unsigned long long) key_len, 
                      password, 
                      (unsigned long long) strlen(password), 
                      salt, 
                      crypto_pwhash_OPSLIMIT_SENSITIVE, 
                      crypto_pwhash_MEMLIMIT_SENSITIVE, 
                      crypto_pwhash_ALG_DEFAULT) != 0) 
    {
        sodium_free(key);
        return NULL;
    }

    return key;
}

// this function generates an high entropy masterkey out of a password
// using an auto-generated random salt that will be stored in "salt" pointer
// so that it can be used to generate the same output at login time. 
unsigned char *generate_masterkey(char *password, unsigned char *salt)
{
    size_t key_len = crypto_kdf_KEYBYTES;
    size_t salt_len = crypto_pwhash_SALTBYTES;

    unsigned char *key = (unsigned char *) sodium_malloc(key_len);
    unsigned char *this_salt = (unsigned char *) sodium_malloc(salt_len);

    if (!this_salt || !key) {
        perror("psm: allocation error");
        sodium_free(key);
        sodium_free(this_salt);
        return NULL;
    }

    randombytes_buf(this_salt, salt_len);

    // this creates the actual hash using the password and the salt (doc.libsodium.org)
    if (crypto_pwhash(key, 
                      (unsigned long long) key_len, 
                      password, 
                      (unsigned long long) strlen(password), 
                      this_salt, 
                      crypto_pwhash_OPSLIMIT_SENSITIVE, 
                      crypto_pwhash_MEMLIMIT_SENSITIVE, 
                      crypto_pwhash_ALG_DEFAULT) != 0) 
    {
        sodium_free(key);
        sodium_free(this_salt);
        return NULL;
    }

    strncpy(salt, this_salt, salt_len);

    sodium_free(this_salt);
    return key;
}

// this function returns an array of "qty" subkeys derived from a masterkey
unsigned char **generate_subkeys(int qty, unsigned char *masterkey)
{
    uint64_t subkey_id = 1;
    uint8_t *subkey;

    size_t subkey_len = crypto_kdf_BYTES_MAX;
    unsigned char **subkeys = (unsigned char **) sodium_malloc(sizeof(char) * subkey_len);

    for (int i=0; i<qty; i++) {

        subkey = (uint8_t *) sodium_malloc(subkey_len);

        // this derives a subkey from the masterkey (doc.libsodium.org)
        if (crypto_kdf_derive_from_key(subkey, subkey_len, subkey_id++, CONTEXT, masterkey) != 0) {
            sodium_free(subkey);
            sodium_free(subkeys);
            return NULL;
        }

        subkeys[i] = subkey;
    }

    sodium_free(subkey);
    return subkeys;
}

// simply writes salt bytes in a file
int write_salt(unsigned char *salt, char *file_path)
{
    size_t wlen;
    size_t salt_len = crypto_pwhash_SALTBYTES;
    FILE *file = fopen(file_path, "w+"); // 'w' opening will automatically clear the file

    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    if ((wlen = fwrite(salt, 1, salt_len, file)) != salt_len) {
        perror("psm: I/O error");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

// simply retrieves salt bytes from a file
int get_salt(unsigned char *salt, char *file_path)
{
    size_t rlen;
    size_t salt_len = crypto_pwhash_SALTBYTES;
    FILE *file = fopen(file_path, "r+");

    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    if ((rlen = fread(salt, 1, salt_len, file)) != salt_len) {
        perror("psm: I/O error");
        fclose(file);
        return -1;
    }

    fclose(file);

    return 0;
}

/*---------------KEY-HANDLING-END---------------*/