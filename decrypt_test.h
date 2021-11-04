#include <stdioplusplus.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define CHUNK_SIZE 4096

/*-----------CONSTANTS-DEFINITION-END-----------*/



/*----------FUNCTIONS-DEFINITION-START----------*/

unsigned char *decrypt(crypto_secretstream_xchacha20poly1305_state *state, unsigned char *buff, size_t *cypher_text_len);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

int decrypt_file(char *file_path, unsigned char *key)
{
    // 1. done - get header
    // 2. done - inti_pull header
    // 3. get cypher text
    // 4. split the cypher text in CHUNK_SIZE size chunks
    // 5. decrypt every chunk and put'em in a final buffer

    unsigned char *header = (unsigned char *) sodium_malloc(crypto_secretstream_xchacha20poly1305_HEADERBYTES);
    crypto_secretstream_xchacha20poly1305_state state;

    size_t rlen;
    size_t header_len = sizeof(header);

    FILE *file = fopen(file_path, "rb");

    if ((rlen = fread(header, 1, header_len, file)) != header_len) {
        perror("psm: I/O error\n");
        return -1;
    }

    if (crypto_secretstream_xchacha20poly1305_init_pull(&state, header, key) != 0) {
        perror("psm: incomplete header\n");
        return -1;
    }

    

}