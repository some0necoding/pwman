#include <string.h>
#include <sodiumplusplus.h>
#include <stdioplusplus.h>

#define CHUNK_SIZE 4096

int encrypt_file(char *file_path, unsigned char *key)
{
    size_t cypher_text_len;
    unsigned char *plain_text;
    unsigned char *crptd_text;
    unsigned char *header = (unsigned char *) sodium_malloc(crypto_secretstream_xchacha20poly1305_HEADERBYTES);
    crypto_secretstream_xchacha20poly1305_state state;

    // check for good allocation
    if (!header) {
        return -1;
    }

    // initialize a state with a key and stores the output in the header
    if (crypto_secretstream_xchacha20poly1305_init_push(&state, header, key) != 0) {
        return -1;
    }

    // get the entire file content
    if (!(plain_text = fgetalls(file_path))) {
        return -1;
    }

    // encrypt the file content
    if (!(crptd_text = encrypt(&state, plain_text, &cypher_text_len))) {
        return -1;
    }
    
    // write the encrypted buff into the file
    if (write_cypher_to_file(header, crptd_text, cypher_text_len, file_path) != 0) {
        return -1;
    }

    return 0;
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

    int big_buff_pos = 0;
    int cnk_buff_pos = 0;
    int ret_buff_pos = 0;

    unsigned char c;
    unsigned long long *out_len; // the length of an out_buff (doc.libsodium.org for more info).

    while ((c = buff[big_buff_pos]) != '\0') {
        // if the cnk_buff is not full continue writing chars into it.
        if (cnk_buff_pos < CHUNK_SIZE) {
            cnk_buff[cnk_buff_pos] = c;
            cnk_buff_pos++;
            big_buff_pos++;
        }
        // if the cnk_buff is full encrypt it and store it in the ret_buff.
        else {
            
            // doc.libsodium.org for more info about this.
            if(crypto_secretstream_xchacha20poly1305_push(state, out_buff, out_len, cnk_buff, cnk_buff_pos, NULL, 0, 0) != 0) {
                return NULL;
            }

            // copying the sub-buffer out_buff to the ret_buff.
            memcpy((void *) ret_buff+ret_buff_pos, out_buff, (size_t) out_len);
            // increasing the ret_buff's "cursor" (== index, it's an array) by the number of bytes contained in the sub-buffer just added.
            ret_buff_pos += (int) out_len;

            // if the ret_buff is not large enough it gets stretched (notice that sodium_realloc is not present in sodium, but it's custom).
            if (ret_buff_pos >= enc_cnk_size) {
                old_buf_size = enc_cnk_size;
                enc_cnk_size += CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES;
                ret_buff = (unsigned char *) sodium_realloc(ret_buff, old_buf_size, enc_cnk_size);
            }

            // cursor position gets zeroed so that the cnk_buff can be refilled from 0 to CHUNK_SIZE again (the buffer is basically emptied,
            // letting new data overwrite the old one).
            cnk_buff_pos = 0;
            // the char that was hanging around since the cnk_buff was full is pushed in the "emptied" cnk_buff.
            cnk_buff[cnk_buff_pos] = c;
            cnk_buff_pos++;
            big_buff_pos++;
        }
    }

    if (crypto_secretstream_xchacha20poly1305_push(state, out_buff, out_len, cnk_buff, cnk_buff_pos, NULL, 0, crypto_secretstream_xchacha20poly1305_TAG_FINAL) != 0) {
        return NULL;
    }

    memcpy((void *) ret_buff+ret_buff_pos, out_buff, (size_t) out_len);
    ret_buff_pos += (int) out_len;

    // if the return buffer is not large enough it gets stretched.
    if (ret_buff_pos >= enc_cnk_size) {
        old_buf_size = enc_cnk_size;
        enc_cnk_size += CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES;
        ret_buff = (unsigned char *) sodium_realloc(ret_buff, old_buf_size, enc_cnk_size);
    }

    // always free memory, mostly if secured, cuz we don't want to have buffers hanging around mlocked.
    sodium_free(cnk_buff);
    sodium_free(out_buff);

    return ret_buff;
}

// the function simply writes some bytes from a buffer to a stream (header + cyhertext -> encrypted file), 
// but with some error checking (always do error checking).
int write_cypher_to_file(unsigned char *header, unsigned char *cypher_text, size_t cypher_text_len, char *file_path)
{
    size_t rlen;
    FILE *file = fopen(file_path, "w");

    if (!file) {
        return -1;
    }

    if ((rlen = fwrite(header, 1, sizeof(header), file)) != sizeof(header)) {
        return -1;
    }

    if ((rlen = fwrite(cypher_text, 1, cypher_text_len, file)) != cypher_text_len) {
        return -1;
    }

    fclose(file);

    return 0;
}

unsigned char *decrypt(unsigned char *key, char *file_path) 
{
    size_t cypher_text_len = CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES;

    unsigned char *buff_in = (unsigned char *) sodium_malloc(CHUNK_SIZE);
    unsigned char *buff_out = (unsigned char *) sodium_malloc(cypher_text_len);
    unsigned char *header = (unsigned char *) sodium_malloc(crypto_secretstream_xchacha20poly1305_HEADERBYTES);
    unsigned char *cypher_text = (unsigned char *) sodium_malloc(cypher_text_len);  
    crypto_secretstream_xchacha20poly1305_state state;

    int eof;
    int pos = 0;
    unsigned char *ret = (unsigned char *) -1;
    size_t rlen;
    size_t old_size;
    unsigned long long out_len;
    unsigned char tag;
    FILE *file = fopen(file_path, "rb");

    fread(header, 1, sizeof header, file);

    if(crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        goto ret; //incomplete header
    }

    do {
        rlen = fread(buff_in, 1, sizeof buff_in, file);
        eof = feof(file);

        if(crypto_secretstream_xchacha20poly1305_pull(&state, buff_out, &out_len, &tag, buff_in, rlen, NULL, 0) != 0) {
            goto ret; // corrupted chunk
        }

        if(tag == crypto_secretstream_xchacha20poly1305_TAG_FINAL && !eof) {
            goto ret; // premature end (end of file reached before the end of the stream)
        }

        memcpy((void *) cypher_text+pos, buff_out, out_len);
    
        pos += out_len;

        if(pos >= cypher_text_len) {
            old_size = cypher_text_len;
            cypher_text_len += CHUNK_SIZE + crypto_secretstream_xchacha20poly1305_ABYTES;
            cypher_text = (unsigned char *) sodium_realloc(cypher_text, old_size, cypher_text_len);
        } 
    } while (!eof);

    ret = cypher_text;

    ret:
        fclose(file);
        return ret;
}