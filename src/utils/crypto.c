#include "./crypto.h"
#include "./input.h"

#include <gpg-error.h>
#include <stddef.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <gpgme.h>
#include <locale.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 512


const char *data_to_buffer(gpgme_data_t dh);
gpgme_error_t passphrase_cb(void *opaque, const char *uid_hint, const char *passphrase_info, int last_was_bad, int fd);
int init_gpgme(gpgme_protocol_t proto);


/*
    This function encrypts buffer plain using openpgp public 
    key of fingerprint fpr. It returns a statically allocated
    buffer containing cyphertext.
*/
const char *gpg_encrypt(const char *plain, const char *fpr) 
{
    gpgme_ctx_t ctx;
    gpgme_error_t err;
    gpgme_data_t in, out;
    gpgme_key_t key[2] = {NULL, NULL};
    gpgme_encrypt_result_t result;

    if (init_gpgme(GPGME_PROTOCOL_OpenPGP) != 0) {
		gpgme_release(ctx);
		return NULL;
	}
     
	if ((err = gpgme_new(&ctx))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_release(ctx);
		return NULL;	
	}

	gpgme_set_armor(ctx, 1);            // output will be ascii armored

    if ((err = gpgme_data_new_from_mem(&in, plain, strlen(plain), 0))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_release(ctx);
		return NULL;	
	}

    if ((err = gpgme_data_new(&out))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_data_release(in);
		gpgme_release(ctx);
		return NULL;	
	}

    if ((err = gpgme_get_key(ctx,fpr,&key[0],0))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_data_release(in);
		gpgme_data_release(out);
		gpgme_release(ctx);
		return NULL;	
	}

	if ((err = gpgme_op_encrypt(ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, in, out))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_data_release(in);
		gpgme_data_release(out);
		gpgme_release(ctx);
		return NULL;	
	}

	result = gpgme_op_encrypt_result(ctx);
    
	if (result->invalid_recipients) {
        fprintf(stderr,"psm:%s:%d: invalid recipient: %s\n", __FILE__, __LINE__, result->invalid_recipients->fpr);
		gpgme_data_release(in);
		gpgme_data_release(out);
		gpgme_release(ctx);
        return NULL;
    }
    
	const char *buffer = data_to_buffer(out);

    if (!buffer) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		gpgme_data_release(in);
		gpgme_data_release(out);
		gpgme_release(ctx);
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
int gpg_decrypt(const char *cypher, const char *fpr, char **buf, size_t bufsize) 
{
    gpgme_ctx_t ctx;
    gpgme_error_t err;
    gpgme_data_t in, out;
    gpgme_encrypt_result_t enc_result;
    gpgme_decrypt_result_t dec_result;
    gpgme_key_t keys[2] = {NULL,NULL};

    size_t plaintext_size;

	int ret_code = -1;

    if (init_gpgme(GPGME_PROTOCOL_OpenPGP) != 0) {
		goto ret;	
	}

    if ((err = gpgme_new(&ctx))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		goto ret;	
	}

    /* 
        If no ssh-agent provides a passphrase to unlock 
        private key a call to passphrase_cb is made. 
    */
    char *agent_info = getenv("GPG_AGENT_INFO");
    
	if (!(agent_info && strchr(agent_info, ':'))) {
        
		if ((err = gpgme_set_pinentry_mode(ctx, GPGME_PINENTRY_MODE_LOOPBACK))) {
			fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
			goto ret;	
		}
        
		gpgme_set_passphrase_cb(ctx, passphrase_cb, NULL);
    }

    /* Search key for encryption. */
    gpgme_key_t key;
    if ((err = gpgme_get_key(ctx,fpr,&key,1))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		goto ret;	
	}

    /* Initialize input buffer. */
    if ((err = gpgme_data_new_from_mem(&in,cypher, strlen(cypher), 0))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		goto ret;	
	}

    /* Initialize output buffer. */
    if ((err = gpgme_data_new(&out))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		goto ret;	
	}

    /* Decrypt data. */
    err = gpgme_op_decrypt(ctx, in, out);

    if (gpgme_err_code(err) == GPG_ERR_BAD_PASSPHRASE) {
		ret_code = 0;	
		goto ret; 
	} else if (err) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		goto ret; 
	}

    dec_result = gpgme_op_decrypt_result(ctx);
    
    if (dec_result->unsupported_algorithm) {
        fprintf(stderr, "%s:%i: unsupported algorithm: %s\n", __FILE__, __LINE__, dec_result->unsupported_algorithm);
		goto ret; 
	}

    const char *plaintext = data_to_buffer(out);

    if (!plaintext) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret; 
	}

    if ((strlen(plaintext) + 1) > bufsize)
        *buf = realloc(*buf, sizeof(char) * (strlen(plaintext) + 1));
        
	if (!*buf) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret; 
	}

    strcpy(*buf, plaintext);

	ret_code = 1;

ret:
    gpgme_data_release(in);
    gpgme_data_release(out);
    gpgme_release(ctx);
    return ret_code;
}


/*
    This function returns an array containing all local 
    gpg keys. Every gpgme_key_t struct can then be accessed
    to retrieve useful data.
*/
gpgme_key_t *gpg_get_keys() 
{
    /* key + NULL */
    size_t keys_size = 2;

    gpgme_ctx_t ctx;
    gpgme_error_t err;
    gpgme_key_t key;
    gpgme_key_t *keys = (gpgme_key_t *) malloc(sizeof(gpgme_key_t) * keys_size);

    int pos = 0;

    if (!keys) {
        fprintf(stderr, "psm:%s:%d: allocation error", __FILE__, __LINE__);
        return NULL;
    }

    if (init_gpgme(GPGME_PROTOCOL_OpenPGP) != 0) {
		return NULL;
	}

    if ((err = gpgme_new(&ctx))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_release(ctx);
		return NULL;	
	}

    /* Start keylist operation */
    if ((err = gpgme_op_keylist_start(ctx, NULL, 1))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		gpgme_release(ctx);
		return NULL;	
	}

    /* Retrieve keys until EOF is reached */
    while ((err = gpgme_op_keylist_next(ctx, &key)) == 0) {

        keys[pos] = key;
        pos++;

        /* Stretch keys array if needed (last index is kept free for NULL) */
        if ((keys_size - 1) <= pos) {
            
            keys_size += 1;
            keys = realloc(keys, (sizeof(gpgme_key_t) * keys_size));

            if (!keys) {
                fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
				gpgme_release(ctx);
                return NULL;
            }
        }

    }

    /* Null terminate keys */
    keys[keys_size - 1] = NULL;

    gpgme_release(ctx);
    return keys;
}


/*
    This is a passphrase callback function used to 
    retrieve private key passphrase from user.

    (https://gnupg.org/documentation/manuals/gpgme/Passphrase-Callback.html#Passphrase-Callback for docs)
*/
gpgme_error_t passphrase_cb(void *opaque, const char *uid_hint, const char *passphrase_info, int last_was_bad, int fd)
{
    int rlen;
    char *pass = (char *) malloc(sizeof(char) * BUF_SIZE);
    int offset = 0;

    printf("Enter passphrase for openpgp key \n\t%s: ", uid_hint);

    /* Read pass with echo disabled (from input_aquisition.h) */
    if (read_line_s(&pass, BUF_SIZE) != 0) {
        fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
        return -1;   
    }

    printf("\n");

    /* Write pass + '\n' to fd */
    do {
        rlen = write(fd, &pass[offset], strlen(pass) - offset);
        if (rlen > 0)
            offset += rlen;
    } while (rlen > 0 && offset < strlen(pass));

    rlen = write(fd, "\n", sizeof(char));

	if (rlen > 0)
        offset += rlen;

    if (offset != strlen(pass))
        gpgme_error_from_errno(errno);

    return 0;
}


/* 
    This function calls a set of functions to 
    set up gpgme.
*/
int init_gpgme(gpgme_protocol_t proto)
{
	gpgme_error_t err;

    gpgme_check_version(NULL);
	setlocale(LC_ALL, "");
	
    if ((err = gpgme_set_locale(NULL, LC_CTYPE, setlocale (LC_CTYPE, NULL)))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		return -1;	
	}
    
    if ((err = gpgme_engine_check_version(proto))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		return -1;	
	}

	return 0;
}


/*
    This function convert a gpgme_data_t object (which
    can contain both cyphertext and plaintext) to a null 
    terminated and statically allocated buffer.
*/
const char *data_to_buffer(gpgme_data_t dh)
{
    size_t buflen = BUF_SIZE;
    char *buffer = (char *) malloc(sizeof(char) * buflen);
    int rlen;
    int last_index = 0;

	gpgme_error_t err;
    
    rlen = gpgme_data_seek(dh, 0, SEEK_SET);
    
    if (rlen && (err = gpgme_err_code_from_errno(errno))) {
		fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
		return NULL;	
	}

    while ((rlen = gpgme_data_read(dh, buffer+last_index, BUF_SIZE)) > 0) {
        
        if (rlen < 0 && (err = gpgme_err_code_from_errno (errno))) {
			fprintf(stderr, "%s:%d: %s: %s\n", __FILE__, __LINE__, gpgme_strsource(err), gpgme_strerror(err));
			return NULL;
        } else if (rlen >= buflen) {

            buflen += BUF_SIZE;
            buffer = realloc(buffer, buflen);

            if  (!buffer) {
                fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
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
    rlen = gpgme_data_seek(dh, 0, SEEK_SET);

    return buffer;
}
