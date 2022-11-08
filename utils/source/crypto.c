#include "../headers/crypto.h"
#include "../headers/input.h"

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <gpgme.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUF_SIZE 512

#define fail_if_err(err)					\
  	do {									\
      	if (err) {							\
          	fprintf (stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource (err), gpgme_strerror (err));			\
          	exit(1);						\
        }									\
    } while (0)

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*----------FUNCTIONS-DEFINITION-START----------*/

const char *data_to_buffer(gpgme_data_t dh);
gpgme_error_t passphrase_cb(void *opaque, const char *uid_hint, const char *passphrase_info, int last_was_bad, int fd);
void init_gpgme(gpgme_protocol_t proto);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

/*
    This function encrypts buffer plain using openpgp public 
    key of fingerprint fpr. It returns a statically allocated
    buffer containing cyphertext.
*/
const char *gpg_encrypt(char *plain, const char *fpr) 
{
    gpgme_ctx_t ctx;
    gpgme_error_t err;
    gpgme_data_t in, out;
    gpgme_key_t key[2] = {NULL, NULL};
    gpgme_encrypt_result_t result;

    init_gpgme(GPGME_PROTOCOL_OpenPGP);
      
    err = gpgme_new(&ctx);
    fail_if_err(err);
    gpgme_set_armor(ctx, 1);            // output will be ascii armored

    err = gpgme_data_new_from_mem(&in, plain, strlen(plain), 0);
    fail_if_err (err);

    err = gpgme_data_new(&out);
    fail_if_err (err);

    err = gpgme_get_key(ctx,fpr,&key[0],0);
    fail_if_err (err);

    err = gpgme_op_encrypt(ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, in, out);
    fail_if_err(err);
    result = gpgme_op_encrypt_result(ctx);
    
	if (result->invalid_recipients) {
        fprintf(stderr,"psm: invalid recipient: %s\n", result->invalid_recipients->fpr);
        exit(1);
    }
    
	const char *buffer = data_to_buffer(out);

    if (!buffer) {
        perror("psm: allocation error\n");
        return NULL;
    }

    gpgme_data_release(in);
    gpgme_data_release(out);
    gpgme_release(ctx);
    return buffer;
}

/*
    This function decrypts buffer cypher using openpgp private
    key of fingerprint fpg. It returns a statically allocated
    buffer containing plaintext.
*/
const char *gpg_decrypt(char *cypher, const char *fpr) 
{
    gpgme_ctx_t ctx;
    gpgme_error_t err;
    gpgme_data_t in,out;
    gpgme_encrypt_result_t enc_result;
    gpgme_decrypt_result_t dec_result;
    gpgme_key_t keys[2] = {NULL,NULL};

    char *agent_info;

    init_gpgme (GPGME_PROTOCOL_OpenPGP);

    err = gpgme_new (&ctx);
    fail_if_err (err);

    /* 
        If no ssh-agent provides a passphrase to unlock 
        private key a call to passphrase_cb is made. 
    */
    agent_info = getenv("GPG_AGENT_INFO");
    if (!(agent_info && strchr(agent_info, ':'))) {
        err = gpgme_set_pinentry_mode(ctx, GPGME_PINENTRY_MODE_LOOPBACK); 
        fail_if_err(err); 
        gpgme_set_passphrase_cb(ctx, passphrase_cb, NULL);
    }

    /* Search key for encryption. */
    gpgme_key_t key;
    err = gpgme_get_key (ctx,fpr,&key,1);
    fail_if_err (err);

    /* Initialize input buffer. */
    err = gpgme_data_new_from_mem (&in,cypher, strlen(cypher), 0);
    fail_if_err (err);

    /* Initialize output buffer. */
    err = gpgme_data_new (&out);
    fail_if_err (err);

    /* Decrypt data. */
    err = gpgme_op_decrypt (ctx, in, out);
    fail_if_err (err);
    dec_result = gpgme_op_decrypt_result (ctx);
    if (dec_result->unsupported_algorithm) {
        fprintf(stderr, "%s:%i: unsupported algorithm: %s\n", __FILE__, __LINE__, dec_result->unsupported_algorithm);
        exit(1);
    }

    const char *buffer = data_to_buffer(out);

    if (!buffer) {
        perror("test: allocation error\n");
        return NULL;
    }

    gpgme_data_release (in);
    gpgme_data_release (out);
    gpgme_release (ctx);
    return buffer;
}

/*
    This is a passphrase callback function used to 
    retrieve private key passphrase from user.

    (https://gnupg.org/documentation/manuals/gpgme/Passphrase-Callback.html#Passphrase-Callback for docs)
*/
gpgme_error_t passphrase_cb(void *opaque, const char *uid_hint, const char *passphrase_info, int last_was_bad, int fd)
{
    int rlen;
    char *pass = calloc(BUF_SIZE, sizeof(char));
    int passlen; 
    int offset = 0;

    printf("Enter passphrase for openpgp key \n\t%s: ", uid_hint);

    /* Read pass with echo disabled (from input_aquisition.h) */
    if (read_line_s(&pass, BUF_SIZE) != 0) {
        perror("psm: allocation error\n");
        return -1;   
    }

    printf("\n");

    passlen = strlen(pass);

    /* Write pass + '\n' to fd */
    do {
        rlen = write(fd, &pass[offset], passlen - offset);
        if (rlen > 0) {
            offset += rlen;
        } 
    } while (rlen > 0 && offset < passlen);

    rlen = write(fd, "\n", sizeof(char));
    if (rlen > 0) {
        offset += rlen;
    }

    if (offset != passlen) {
        gpgme_error_from_errno(errno);
    }

    return 0;
}

/* 
    This function calls a set of functions to 
    set up gpgme.
*/
void init_gpgme(gpgme_protocol_t proto)
{
	gpgme_error_t err;

    gpgme_check_version(NULL);
	setlocale(LC_ALL, "");
	
    err = gpgme_set_locale(NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
    fail_if_err(err);
    
    err = gpgme_engine_check_version(proto);
	fail_if_err(err);
}

/*
    This function convert a gpgme_data_t object (which
    can contain both cyphertext and plaintext) to a null 
    terminated and statically allocated buffer.
*/
const char *data_to_buffer(gpgme_data_t dh)
{
    size_t buflen = BUF_SIZE;
    char *buffer = calloc(buflen, sizeof(char));
    int rlen;
    int last_index = 0;
    
    rlen = gpgme_data_seek (dh, 0, SEEK_SET);
    
    if (rlen) {
        fail_if_err (gpgme_err_code_from_errno (errno));
    }

    while ((rlen = gpgme_data_read (dh, buffer+last_index, BUF_SIZE)) > 0) {
        
        if (rlen < 0) {
            fail_if_err (gpgme_err_code_from_errno (errno));
        } else if (rlen >= buflen) {

            buflen += BUF_SIZE;
            buffer = realloc(buffer,  buflen);

            if  (!buffer) {
                perror("psm: allocation error\n");
                return NULL;
            }
        }

        last_index += rlen;
    }

    /* 
        Reset read position to the beginning so that dh can be used as input
        for another operation after this method call. For example, dh is an
        output from encryption and also is used as an input for decryption.
        Otherwise GPG_ERR_NO_DATA is returned since this method moves the
        read position. 
    */
    rlen = gpgme_data_seek (dh, 0, SEEK_SET);

    return buffer;
}