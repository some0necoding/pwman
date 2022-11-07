#include "../headers/cryptography.h"

//#include <stddef.h>
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
void init_gpgme(gpgme_protocol_t proto);

/*-----------FUNCTIONS-DEFINITION-END-----------*/

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
    gpgme_set_armor(ctx, 1);

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
    }

    delete_test_key(ctx,key[0]);
    gpgme_data_release(in);
    gpgme_data_release(out);
    gpgme_release(ctx);
    return 0;
}

void init_gpgme(gpgme_protocol_t proto)
{
	gpgme_error_t err;

	gpgme_check_version(NULL);
	setlocale(LC_ALL, "");
	gpgme_set_locale(NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL));
	err = gpgme_engine_check_version(proto);
	fail_if_err(err);
}

const char *data_to_buffer(gpgme_data_t dh)
{
    size_t buflen = BUF_SIZE;
    char *buffer = calloc(buflen, sizeof(char));
    int rlen;
    
    rlen = gpgme_data_seek (dh, 0, SEEK_SET);
    
    if (rlen) {
        fail_if_err (gpgme_err_code_from_errno (errno));
    }

    while ((rlen = gpgme_data_read (dh, buffer, BUF_SIZE)) > 0) {
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