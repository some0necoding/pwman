#include "../headers/cryptography.h"

#include "../headers/sodiumplusplus.h"
#include "../headers/stdioplusplus.h"

#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define CHUNK_SIZE 4096
#define CYPHER_TEXT_SIZE 512
#define CONTEXT "decrypt_"
#define HEADER_LENGTH crypto_secretstream_xchacha20poly1305_HEADERBYTES

/*-----------CONSTANTS-DEFINITION-END-----------*/



/*----------FUNCTIONS-DEFINITION-START----------*/

int encrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t *cypher_text_len, unsigned char *ret_buff, size_t ret_buff_size);
int decrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t cypher_text_len, unsigned char *ret_buff, size_t ret_buff_size);
int write_cypher_to_file(unsigned char *header, unsigned char *cypher_text, size_t cypher_text_len, char *file_path);

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
    size_t header_len = HEADER_LENGTH;
    size_t cypher_text_len = CYPHER_TEXT_SIZE;

    unsigned char *cypher_text = (unsigned char *) sodium_malloc(cypher_text_len);
    unsigned char *header = (unsigned char *) sodium_malloc(header_len);

    int ret_code = -1;
    
    crypto_secretstream_xchacha20poly1305_state state;

    // check for good allocation
    if (!header | !cypher_text) {
        perror("psm: allocation error");
        header ? sodium_free(header) : 0;
        cypher_text ? sodium_free(cypher_text) : 0;
        return -1;
    }

    // initialize a state with a key and stores the output in the header
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        perror("psm: header init failed");
        goto ret;
    }

    // encrypt the buffer
    if (encrypt(&state, plain_buff, &cypher_text_len, cypher_text, cypher_text_len) != 0) {
        goto ret;
    }
    
    // write the encrypted buff into the file
    if (write_cypher_to_file(header, cypher_text, cypher_text_len, file_path) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(header);
    sodium_free(cypher_text);
    return ret_code;
}

// To make the header file more complete there's also an encryption function decrypted file -> encrypted file.
// It can be useful in cases were the machine and storage are not easily vulnerable (don't be confident about 
// this choice).
int encrypt_file(char *file_path, unsigned char *key)
{
    size_t header_len = HEADER_LENGTH;
    size_t cypher_text_len = CYPHER_TEXT_SIZE;
    size_t plain_text_len = CHUNK_SIZE;

    unsigned char *plain_text = (unsigned char *) sodium_malloc(plain_text_len);
    unsigned char *cypher_text = (unsigned char *) sodium_malloc(cypher_text_len);
    unsigned char *header = (unsigned char *) sodium_malloc(header_len);
    
    int ret_code = -1;
    
    crypto_secretstream_xchacha20poly1305_state state;

    // check for good allocation
    if (!header | !plain_text | !cypher_text) {
        perror("psm: allocation error");
        header ? sodium_free(header) : 0;
        plain_text ? sodium_free(plain_text) : 0;
        cypher_text ? sodium_free(cypher_text) : 0;
        return -1;
    }

    // initialize a state with a key and stores the output in the header
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        perror("psm: header init failed");
        goto ret;
    }

    // get the entire file content
    if (fgetalls(file_path, plain_text, plain_text_len) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    // encrypt the file content
    if (encrypt(&state, plain_text, &cypher_text_len, cypher_text, cypher_text_len) != 0) {
        goto ret;
    }

    // write the encrypted buff into the file
    if (write_cypher_to_file(header, cypher_text, cypher_text_len, file_path) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    ret_code = 0;

ret:
    sodium_free(header);
    sodium_free(plain_text);
    sodium_free(cypher_text);
    return ret_code;
}

// this function encrypts a buffer using the libsodium library, in order to understand well the code you should be aware of what's inside 
// its documentation (doc.libsodium.org). 
// Basically this function will split up the buffer into smaller buffers of CHUNK_SIZE size, it will then encrypt them one by one 
// and lastly it will store them in a bigger buffer called ret_buff that will be returned.
int encrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t *cypher_text_len, unsigned char *ret_buff, size_t ret_buff_size)
{
    size_t old_buf_size;
    size_t dec_cnk_size = CHUNK_SIZE;
    size_t enc_cnk_size = CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES; // doc.libsodium.org for more info about this.

    unsigned char *cnk_buff = (unsigned char *) sodium_malloc(dec_cnk_size); // sub-buffer of the plaintext (doc.libsodium.org for more info).
    unsigned char *out_buff = (unsigned char *) sodium_malloc(enc_cnk_size); // this buffer contains the encrypted cnk_buff.

    int big_buff_pos = 0;
    int cnk_buff_pos = 0;
    int ret_buff_pos = 0;

    int ret_code = -1;

    unsigned char c;
    unsigned long long out_len; // the length of an out_buff (doc.libsodium.org for more info).

    if (!cnk_buff | !out_buff) {
        perror("psm: allocation error");
        cnk_buff ? sodium_free(cnk_buff) : 0;
        out_buff ? sodium_free(out_buff) : 0;
        return -1;
    }

    if (ret_buff_size < enc_cnk_size) {
        ret_buff = (unsigned char *) sodium_realloc(ret_buff, ret_buff_size, enc_cnk_size);

        if (!ret_buff) {
            perror("psm: allocation error");
            sodium_free(cnk_buff);
            sodium_free(out_buff);
            return -1;
        }
    }

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
                
                if (!ret_buff) {
                    perror("psm: allocation error");
                    goto ret;
                }
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

    ret_code = 0;

ret:
    // always free memory, mostly if secured, cuz we don't wanna have buffers hanging around mlocked.
    sodium_free(cnk_buff);
    sodium_free(out_buff);
    sodium_free(ret_buff);
    return ret_code;
}

// the function simply writes some bytes from a buffer to a stream 
// (cyphertext_len + header + cyhertext -> encrypted file).
int write_cypher_to_file(unsigned char *header, unsigned char *cypher_text, size_t cypher_text_len, char *file_path)
{
    size_t rlen;
    size_t size_t_len = sizeof(size_t);
    size_t header_len = HEADER_LENGTH;

    int ret_code = -1;

    FILE *file = fopen(file_path, "w");

    if (!file) {
        perror("psm: allocation error");
        return -1;
    }

    if ((rlen = fwrite(&cypher_text_len, size_t_len, 1, file)) != 1) {
        perror("psm: I/O error");
        goto ret;
    }

    if ((rlen = fwrite(header, 1, header_len, file)) != header_len) {
        perror("psm: I/O error");
        goto ret;
    }

    if ((rlen = fwrite(cypher_text, 1, cypher_text_len, file)) != cypher_text_len) {
        perror("psm: I/O error1");
        goto ret;
    }

    ret_code = 0;

ret:
    fclose(file);
    return ret_code;
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
int decrypt_file(char *file_path, unsigned char *key, unsigned char *plain_text, size_t plain_text_size)
{
    size_t rlen;
    size_t size_t_len = sizeof(size_t);
    size_t header_len = HEADER_LENGTH;
    size_t cypher_text_len = 0;

    unsigned char *cypher_text;                                                 // this will get allocated later on
    unsigned char *header = (unsigned char *) sodium_malloc(header_len);

    int ret_code = -1;

    crypto_secretstream_xchacha20poly1305_state state;

    FILE *file = fopen(file_path, "rb");

    if (!file) {
        perror("psm: I/O error");
        header ? sodium_free(header) : NULL;
        return -1;
    }

    if (!header) {
        perror("psm: allocation error");
        fclose(file);
        return -1;
    }

    if ((rlen = fread(&cypher_text_len, size_t_len, 1, file)) != 1) {
        perror("psm: I/O error");
        fclose(file);
        sodium_free(header);
        return -1;
    }

    cypher_text = (unsigned char *) sodium_malloc(cypher_text_len);

    if (!cypher_text) {
        perror("psm: allocation error");
        fclose(file);
        sodium_free(header);
        return -1;
    }

    if (fgetfromtos(file_path, size_t_len, (header_len + size_t_len), header, header_len) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        perror("psm: incomplete header");
        goto ret;
    }

    if (fgetfromtos(file_path, (header_len + size_t_len), (cypher_text_len + header_len + size_t_len), cypher_text, cypher_text_len) != 0) {
        perror("psm: I/O error");
        goto ret;
    }

    if (decrypt(&state, cypher_text, cypher_text_len, plain_text, plain_text_size) != 0) {
        goto ret;
    }

    ret_code = 0;

ret:
    fclose(file);
    sodium_free(header);
    sodium_free(cypher_text);
    return ret_code;
}

// this function decrypts a buffer using the libsodium library, in order to understand well the code you should be aware of what's inside 
// its documentation (doc.libsodium.org). 
// Basically this function will split up the buffer into smaller buffers of CHUNK_SIZE size, it will then decrypt them one by one 
// and lastly it will store them in a bigger buffer called ret_buff that will be returned.
int decrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t buff_len, unsigned char *ret_buff, size_t ret_buff_size)
{
    size_t old_buf_size;
    size_t dec_cnk_size = CHUNK_SIZE;
    size_t enc_cnk_size = CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES; // doc.libsodium.org for more info about this.

    unsigned char *cnk_buff = (unsigned char *) sodium_malloc(enc_cnk_size); // sub-buffer of the cyphertext (doc.libsodium.org for more info).
    unsigned char *out_buff = (unsigned char *) sodium_malloc(dec_cnk_size); // this buffer contains the decrypted cnk_buff.

    unsigned char tag;

    int big_buff_pos = 0;
    int cnk_buff_pos = 0;
    int ret_buff_pos = 0;

    int ret_code = -1;

    unsigned long long out_len; // the length of an out_buff (doc.libsodium.org for more info).

    if (!cnk_buff | !out_buff) {
        perror("psm: allocation error");
        cnk_buff ? sodium_free(cnk_buff) : 0;
        out_buff ? sodium_free(out_buff) : 0;
        return -1;
    }

    if (ret_buff_size < dec_cnk_size) {
        ret_buff = (unsigned char *) sodium_realloc(ret_buff, ret_buff_size, dec_cnk_size);

        if (!ret_buff) {
            perror("psm: allocation error");
            sodium_free(cnk_buff);
            sodium_free(out_buff);
            return -1;
        }
    }

    while (big_buff_pos < buff_len) {
        // if the cnk_buff is not full continue writing bytes into it.
        if (cnk_buff_pos < enc_cnk_size) {
            cnk_buff[cnk_buff_pos] = buff[big_buff_pos];
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

                if (!ret_buff) {
                    perror("psm: allocation error");
                    goto ret;
                }
            }

            // cursor position gets zeroed so that cnk_buff can be refilled from 0 to CHUNK_SIZE again (the buffer is basically emptied,
            // letting new data overwrite the old one).
            cnk_buff_pos = 0;
        }
    }

    if (crypto_secretstream_xchacha20poly1305_pull(state, out_buff, &out_len, &tag, cnk_buff, cnk_buff_pos, NULL, 0) != 0) {
        perror("psm: decryption: corrupted chunk");
        goto ret;
    }

    memcpy((void *) ret_buff+ret_buff_pos, (void *) out_buff, (size_t) out_len);
    ret_buff_pos += (int) out_len;

    // copying ret_buff into ret_arr
    ret_code = 0;

ret:
    // always free memory, mostly if secured, cuz we don't wanna have buffers hanging around mlocked.
    sodium_free(cnk_buff);
    sodium_free(out_buff);
    sodium_free(ret_buff);
    return ret_code;
}

/*----------------DECRYPTION-END----------------*/



/*--------------KEY-HANDLING-START--------------*/

// this function generates an high entropy masterkey out of a password and
// a user-provided salt and stores it in "ret_mkey"
int generate_masterkey_with_salt(char *password, unsigned char *salt, unsigned char *ret_mkey, size_t ret_mkey_size)
{
    size_t key_len = crypto_kdf_KEYBYTES;

    if (ret_mkey_size < key_len) {
        ret_mkey = (unsigned char *) sodium_realloc(ret_mkey, ret_mkey_size, key_len);

        if (!ret_mkey) {
            perror("psm: allocation error");
            return -1;
        }
    }

    // this creates the actual hash using the password and the salt (doc.libsodium.org)
    if (crypto_pwhash(ret_mkey, 
                      (unsigned long long) key_len, 
                      password, 
                      (unsigned long long) strlen(password), 
                      salt, 
                      crypto_pwhash_OPSLIMIT_SENSITIVE, 
                      crypto_pwhash_MEMLIMIT_SENSITIVE, 
                      crypto_pwhash_ALG_DEFAULT) != 0) 
    {
        return -1;
    }

    return 0;
}

// this function generates an high entropy masterkey out of a password
// using an auto-generated random salt that will be stored in "salt" pointer
// so that it can be used to generate the same output at login time. 
int generate_masterkey(char *password, unsigned char *salt, unsigned char *ret_mkey, size_t ret_mkey_size)
{
    size_t key_len = crypto_kdf_KEYBYTES;
    size_t salt_len = crypto_pwhash_SALTBYTES;

    unsigned char *this_salt = (unsigned char *) sodium_malloc(salt_len);

    int ret_code = -1;

    if (!this_salt) {
        perror("psm: allocation error");
        return -1;
    }

    if (ret_mkey_size < key_len) {
        ret_mkey = (unsigned char *) sodium_realloc(ret_mkey, ret_mkey_size, key_len);

        if (!ret_mkey) {
            perror("psm: allocation error");
            return -1;
        }
    }

    randombytes_buf(this_salt, salt_len);

    // this creates the actual hash using the password and the salt (doc.libsodium.org)
    if (crypto_pwhash(ret_mkey, 
                      (unsigned long long) key_len, 
                      password, 
                      (unsigned long long) strlen(password), 
                      this_salt, 
                      crypto_pwhash_OPSLIMIT_SENSITIVE, 
                      crypto_pwhash_MEMLIMIT_SENSITIVE, 
                      crypto_pwhash_ALG_DEFAULT) != 0) 
    {
        goto ret;
    }

    strncpy(salt, this_salt, salt_len);

    ret_code = 0;

ret:
    sodium_free(this_salt);
    return ret_code;
}

// this function returns an array of "qty" subkeys derived from a masterkey
int generate_subkeys(int qty, unsigned char *masterkey, unsigned char **subkeys, size_t subkeys_size, size_t subkey_size)
{
    size_t subkey_len = crypto_kdf_BYTES_MAX;

    uint64_t subkey_id = 1;
    uint8_t *subkey;

    if (subkeys_size < (qty * subkey_len)) {
        subkeys = (unsigned char **) sodium_realloc(subkeys, subkeys_size, (qty * subkey_len));

        if (!subkeys) {
            perror("psm: allocation error");
            return -1;
        }
    }

    if (subkey_size < subkey_len) {
        for (int i=0; i<qty; i++) {
            subkeys[i] = (unsigned char *) sodium_realloc(subkeys[i], subkey_size, subkey_len);

            if (!subkeys[i]) {
                perror("psm: allocation error");

                for (int j=0; j<qty; j++) {
                    (j != i) ? sodium_free(subkeys[j]) : 0;
                }

                sodium_free(subkeys);
            }
        }
    }

    for (int i=0; i<qty; i++) {

        subkey = (uint8_t *) sodium_malloc(subkey_len);

        if (!subkey) {
            sodium_free_2d_arr(qty, subkeys);
            return -1;
        }

        // this derives a subkey from the masterkey (doc.libsodium.org)
        if (crypto_kdf_derive_from_key(subkey, subkey_len, subkey_id++, CONTEXT, masterkey) != 0) {
            sodium_free(subkey);
            sodium_free_2d_arr(qty, subkeys);
            return -1;
        }

        memcpy(subkeys[i], subkey, subkey_len);

        sodium_free(subkey);
    }

    sodium_free_2d_arr(qty, subkeys);
    return 0;
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