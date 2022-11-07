#include "../../utils/headers/cryptography.h"

#include <gpg-error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) 
{
    const char *GPG_ID = "2FDC14FAD4542E966B17C8F2558093477E4CB481";
    const char *cyphertext = gpg_encrypt("ciao", GPG_ID); 

    if (!cyphertext) {
        perror("test: allocation error");
        return -1;
    }

    printf("test: cyphertext: %s\n", cyphertext);

    const char *plaintext = gpg_decrypt((char *)cyphertext, GPG_ID);

    if (!plaintext) {
        perror("test: allocation error");
        return -1;
    }

    printf("test: plaintext: %s\n", plaintext);

    return 0;
}