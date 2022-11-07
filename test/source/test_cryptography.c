#include "../../utils/headers/cryptography.h"

#include <gpg-error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char const *argv[]) {

    const char *text = gpg_encrypt("ciao", "2FDC14FAD4542E966B17C8F2558093477E4CB481");
    int ch;

    if (text) {
        printf("%s\n", text);
    } else {
        perror("test: allocation error");
        return -1;
    }

    free((char *) text);
    return 0;
}