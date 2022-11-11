#include "../../utils/headers/crypto.h"

#include <stdio.h>
#include <stdlib.h>
#include <gpgme.h>

int main(int argc, char const *argv[]) {

    printf("test: test start\n");

    printf("test: calling functions\n");

    gpgme_key_t *keys = gpg_get_keys();
    gpgme_key_t key;

    int pos = 0;

    while ((key = keys[pos])) {

        char *fpr = key->fpr;
        char *uid = key->uids->uid;

        printf("fpr : %s\n", fpr);
        printf("uid: %s\n", uid);

        pos++;
    }

    printf("test: test end\n");

    return 0;
}