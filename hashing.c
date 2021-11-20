#ifndef SODIUM_PLUS_PLUS
    #include <sodiumplusplus.h>
#endif

/*----------CONSTANTS-DEFINITION-START----------*/

/*-----------CONSTANTS-DEFINITION-END-----------*/



/*----------FUNCTIONS-DEFINITION-START----------*/

/*-----------FUNCTIONS-DEFINITION-END-----------*/

int pass_hash(char *pass, size_t pass_len) {

    char *out = (char *) sodium_malloc(crypto_pwhash_STRBYTES);

    if (crypto_pwhash_str(out, pass, (unsigned long long) pass_len, 3, ))

}