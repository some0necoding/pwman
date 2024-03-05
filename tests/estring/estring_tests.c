#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/utils/estring.h"

void estrcat_test();
void estrncat_test();
void esstrcat_test();
void esstrncat_test();

int main() {

    printf("== RUN -- estrncat_test()\n");
    estrncat_test();
    printf("== PASS\n");

    printf("== RUN -- estrcat()\n");
    estrcat_test();
    printf("== PASS\n");

    printf("== RUN -- esstrncat()\n");
    esstrncat_test();
    printf("== PASS\n");

    printf("== RUN -- esstrcat()\n");
    esstrcat_test();
    printf("== PASS\n");

}

void estrncat_test() {
    const char TEST_STR[] = "ciao";

    // test every possible length up to strlen(TEST_STR) both with prefix and 
    // empty string.
    for (int i = 0; i <= strlen(TEST_STR); i++) {
        char const* src = strdup(TEST_STR);

        // prefixed dst
        char nempty_dst[10] = "test";
        size_t prefixlen = strlen(nempty_dst);
        char* nempty_end = estrncat(nempty_dst + prefixlen, src, i);

        // empty dst
        char empty_dst[5];
        char* empty_end = estrncat(empty_dst, src, i);

        // check that src remains unmodified
        assert(strcmp(src, TEST_STR) == 0); 

        // check that characters match
        for (int j = 0; j < i; j++) {
            assert(src[j] == empty_dst[j]);
            assert(src[j] == nempty_dst[prefixlen + j]);
        }

        // check that end is actually the first null byte after dst
        assert(empty_end == (empty_dst + i) && *empty_end == 0);
        assert(nempty_end == (nempty_dst + prefixlen + i) && *nempty_end == 0);
    }
}

void estrcat_test() {
    const char TEST_STR[] = "ciao";
    char const* src = strdup(TEST_STR);
    size_t srclen = strlen(src);

    // prefixed dst
    char nempty_dst[10] = "test";
    size_t prefixlen = strlen(nempty_dst);
    char* nempty_end = estrcat(nempty_dst + prefixlen, src);

    // empty dst
    char empty_dst[5];
    char* empty_end = estrcat(empty_dst, src);

    // check that src remains unmodified
    assert(strcmp(src, TEST_STR) == 0); 

    // check that characters match
    for (int j = 0; j < srclen; j++) {
        assert(src[j] == empty_dst[j]);
        assert(src[j] == nempty_dst[prefixlen + j]);
    }

    // check that end is actually the first null byte after dst
    assert(empty_end == (empty_dst + srclen) && *empty_end == 0);
    assert(nempty_end == (nempty_dst + prefixlen + srclen) && *nempty_end == 0);
}

void esstrncat_test() {
    const char TEST_STR[] = "longerstring";

    // test without overflow every possible length up to strlen(TEST_STR) both 
    // with prefix and empty string.
    for (int i = 0; i <= strlen(TEST_STR); i++) {
        char const* src = strdup(TEST_STR);

        // prefixed dst
        size_t SIZE = 32;
        char* nempty_dst = (char *) calloc(SIZE, sizeof(char));
        estrcat(nempty_dst, "prefix");
        size_t prefixlen = strlen(nempty_dst);
        char* nempty_end = esstrncat(&nempty_dst, nempty_dst + prefixlen, &SIZE, src, i);

        // empty dst
        char* empty_dst = (char *) calloc(SIZE, sizeof(char));
        char* empty_end = esstrncat(&empty_dst, empty_dst, &SIZE, src, i);

        // check that src remains unmodified
        assert(strcmp(src, TEST_STR) == 0); 

        // check that characters match
        for (int j = 0; j < i; j++) {
            assert(src[j] == empty_dst[j]);
            assert(src[j] == nempty_dst[prefixlen + j]);
        }

        // check that end is actually the first null byte after dst
        assert(empty_end == (empty_dst + i) && *empty_end == 0);
        assert(nempty_end == (nempty_dst + prefixlen + i) && *nempty_end == 0);
    }

    // test with overflow every possible length up to strlen(TEST_STR) both 
    // with prefix and empty string.
    for (int i = 0; i <= strlen(TEST_STR); i++) {
        char const* src = strdup(TEST_STR);

        // prefixed dst
        size_t SIZE = 8;
        char* nempty_dst = (char *) calloc(SIZE, sizeof(char));
        estrcat(nempty_dst, "prefix");
        size_t prefixlen = strlen(nempty_dst);
        char* nempty_end = esstrncat(&nempty_dst, nempty_dst + prefixlen, &SIZE, src, i);

        // empty dst
        char* empty_dst = (char *) calloc(SIZE, sizeof(char));
        char* empty_end = esstrncat(&empty_dst, empty_dst, &SIZE, src, i);

        // check that src remains unmodified
        assert(strcmp(src, TEST_STR) == 0); 

        // check that characters match
        for (int j = 0; j < i; j++) {
            assert(src[j] == empty_dst[j]);
            assert(src[j] == nempty_dst[prefixlen + j]);
        }

        // check that end is actually the first null byte after dst
        assert(empty_end == (empty_dst + i) && *empty_end == 0);
        assert(nempty_end == (nempty_dst + prefixlen + i) && *nempty_end == 0);
    }
}

void esstrcat_test() {
    const char TEST_STR[] = "longerstring";
    const char PREFIX[] = "prefix";
    size_t prefixlen = strlen(PREFIX);

    char const* src = strdup(TEST_STR);
    size_t srclen = strlen(src);

    // prefixed dst without overflow
    size_t SIZE_NOF = 32;
    char* nempty_nof_dst = (char *) calloc(SIZE_NOF, sizeof(char));
    estrcat(nempty_nof_dst, PREFIX);
    char* nempty_nof_end = esstrcat(nempty_nof_dst, nempty_nof_dst + prefixlen, &SIZE_NOF, src);

    // prefixed dst with overflow
    size_t SIZE_OF = 8;
    char* nempty_of_dst = (char *) calloc(SIZE_OF, sizeof(char));
    estrcat(nempty_of_dst, PREFIX);
    char* nempty_of_end = esstrcat(nempty_of_dst, nempty_of_dst + prefixlen, &SIZE_OF, src);

    // empty dst without overflow
    char* empty_nof_dst = (char *) calloc(SIZE_NOF, sizeof(char));
    char* empty_nof_end = esstrcat(empty_nof_dst, empty_nof_dst, &SIZE_NOF, src);

    // empty dst with overflow
    char* empty_of_dst = (char *) calloc(SIZE_OF, sizeof(char));
    char* empty_of_end = esstrcat(empty_of_dst, empty_of_dst, &SIZE_OF, src);

    // check that src remains unmodified
    assert(strcmp(src, TEST_STR) == 0); 

    // check that characters match
    for (int j = 0; j < srclen; j++) {
        assert(src[j] == empty_nof_dst[j]);
        assert(src[j] == empty_of_dst[j]);
        assert(src[j] == nempty_nof_dst[prefixlen + j]);
        assert(src[j] == nempty_of_dst[prefixlen + j]);
    }

    // check that end is actually the first null byte after dst
    assert(empty_nof_end == (empty_nof_dst + srclen) && *empty_nof_end == 0);
    assert(empty_of_end == (empty_of_dst + srclen) && *empty_of_end == 0);
    assert(nempty_nof_end == (nempty_nof_dst + prefixlen + srclen) && *nempty_nof_end == 0);
    assert(nempty_of_end == (nempty_of_dst + prefixlen + srclen) && *nempty_of_end == 0);
}
