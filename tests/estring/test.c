#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include "../../src/utils/estring.h"

#define BUF_SIZE 64

void estrcat_test();
void estrncat_test();
void esstrcat_test();
void esstrncat_test();
void trimcr_test();
void trimcl_test();
void trimc_test();
void trimr_test();
void triml_test();
void trim_test();

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

    printf("== RUN -- trimcr()\n");
    trimcr_test();
    printf("== PASS\n");

    printf("== RUN -- trimcl()\n");
    trimcl_test();
    printf("== PASS\n");

    printf("== RUN -- trimc()\n");
    trimc_test();
    printf("== PASS\n");

    printf("== RUN -- trimr()\n");
    trimr_test();
    printf("== PASS\n");

    printf("== RUN -- triml()\n");
    triml_test();
    printf("== PASS\n");

    printf("== RUN -- trim()\n");
    trim_test();
    printf("== PASS\n");

}

void estrncat_test() {
    const char TEST_STR[] = "ciao";
    size_t const TEST_STR_LEN = strlen(TEST_STR);
    size_t const OVERFLOW_OFFSET = 10;

    // test every possible length up to strlen(TEST_STR) both with prefix and 
    // empty string.
    for (uint i = 0; i <= TEST_STR_LEN + OVERFLOW_OFFSET; i++) {
        char const* src = strdup(TEST_STR);
        uint i_cap = MIN(i, TEST_STR_LEN); 

        // prefixed dst
        char nempty_dst[10] = "test";
        size_t prefixlen = strlen(nempty_dst);
        char* nempty_end = estrncat(nempty_dst + prefixlen, src, i);

        // empty dst
        char empty_dst[5] = "";
        char* empty_end = estrncat(empty_dst, src, i);

        // check that src remains unmodified
        assert(strcmp(src, TEST_STR) == 0); 

        // check that characters match
        assert(strncmp(src, empty_dst, i_cap) == 0);
        assert(strncmp(src, nempty_dst + prefixlen, i_cap) == 0);

        // check that dst is null-terminated
        assert(*empty_end  == 0);
        assert(*nempty_end == 0);

        // check that end is actually at the end of dst
        assert(empty_end  == (empty_dst + i_cap));
        assert(nempty_end == (nempty_dst + prefixlen + i_cap));
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
    char empty_dst[5] = "";
    char* empty_end = estrcat(empty_dst, src);

    // check that src remains unmodified
    assert(strcmp(src, TEST_STR) == 0); 

    // check that characters match
    assert(strcmp(src, empty_dst) == 0);
    assert(strcmp(src, nempty_dst + prefixlen) == 0);

    // check that dst is null-terminated
    assert(*empty_end  == 0);
    assert(*nempty_end == 0);

    // check that end is actually at the end of dst
    assert(empty_end  == (empty_dst + srclen));
    assert(nempty_end == (nempty_dst + prefixlen + srclen));
}

void esstrncat_test() {
    const char TEST_STR[] = "longerstring";
    size_t const TEST_STR_LEN = strlen(TEST_STR);
    size_t const OVERFLOW_OFFSET = 10;

    // test without overflow every possible length up to strlen(TEST_STR) both 
    // with prefix and empty string.
    for (int i = 0; i <= TEST_STR_LEN + OVERFLOW_OFFSET; i++) {
        char const* src = strdup(TEST_STR);
        uint i_cap = MIN(i, TEST_STR_LEN); 

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
        assert(strncmp(src, empty_dst, i_cap) == 0);
        assert(strncmp(src, nempty_dst + prefixlen, i_cap) == 0);

        // check that dst is null-terminated
        assert(*empty_end  == 0);
        assert(*nempty_end == 0);

        // check that end is actually at the end of dst
        assert(empty_end  == (empty_dst  + i_cap));
        assert(nempty_end == (nempty_dst + prefixlen + i_cap));
    }

    // test with overflow every possible length up to strlen(TEST_STR) both 
    // with prefix and empty string.
    for (int i = 0; i <= TEST_STR_LEN + OVERFLOW_OFFSET; i++) {
        char const* src = strdup(TEST_STR);
        uint i_cap = MIN(i, TEST_STR_LEN); 

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
        assert(strncmp(src, empty_dst, i_cap) == 0);
        assert(strncmp(src, nempty_dst + prefixlen, i_cap) == 0);

        // check that dst is null-terminated
        assert(*empty_end  == 0);
        assert(*nempty_end == 0);

        // check that end is actually at the end of dst
        assert(empty_end  == (empty_dst  + i_cap));
        assert(nempty_end == (nempty_dst + prefixlen + i_cap));
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
    assert(strcmp(src, empty_nof_dst) == 0);
    assert(strcmp(src, empty_of_dst)  == 0);
    assert(strcmp(src, nempty_nof_dst + prefixlen) == 0);
    assert(strcmp(src, nempty_of_dst  + prefixlen) == 0);

    // check that dst is null-terminated
    assert(*empty_nof_end  == 0);
    assert(*empty_of_end   == 0);
    assert(*nempty_nof_end == 0);
    assert(*nempty_of_end  == 0);

    // check that end is actually at the end of dst
    assert(empty_nof_end  == (empty_nof_dst  + srclen));
    assert(empty_of_end   == (empty_of_dst   + srclen));
    assert(nempty_nof_end == (nempty_nof_dst + prefixlen + srclen));
    assert(nempty_of_end  == (nempty_of_dst  + prefixlen + srclen));
}

void trimcr_test() {

    char* HEAP_TEST_STR_1 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_2 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_3 = calloc(BUF_SIZE, sizeof(char));

    strcpy(HEAP_TEST_STR_1, "xxxxstring");
    strcpy(HEAP_TEST_STR_2, "stringxxxx");
    strcpy(HEAP_TEST_STR_3, "xxxxstringxxxx");

    char* HEAP_TEST_STRS[] = {
        HEAP_TEST_STR_1,
        HEAP_TEST_STR_2,
        HEAP_TEST_STR_3,
        NULL,
    };

    char* const HEAP_EXPECTED[] = {
        "xxxxstring",
        "string",
        "xxxxstring",
    };

    char TEST_STRS[][20] = {
        "xxxxstringxx",
        "string",
        "",
        "xxstring",
        "stringxxxx",
    };

    char* const EXPECTED[] = {
        "xxxxstring",
        "string",
        "",
        "xxstring",
        "string",
    };

    int const N_TEST_STRS = sizeof(TEST_STRS) / 20;

    for (int i = 0; HEAP_TEST_STRS[i]; i++) {
        trimcr(HEAP_TEST_STRS[i], 'x');
        assert(strcmp(HEAP_EXPECTED[i], HEAP_TEST_STRS[i]) == 0);
    }

    for (int i = 0; i < N_TEST_STRS; i++) {
        trimcr(TEST_STRS[i], 'x');
        assert(strcmp(EXPECTED[i], TEST_STRS[i]) == 0);
    }

    free(HEAP_TEST_STR_1);
    free(HEAP_TEST_STR_2);
    free(HEAP_TEST_STR_3);    
}

void trimcl_test() {

    char* HEAP_TEST_STR_1 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_2 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_3 = calloc(BUF_SIZE, sizeof(char));

    strcpy(HEAP_TEST_STR_1, "xxxxstring");
    strcpy(HEAP_TEST_STR_2, "stringxxxx");
    strcpy(HEAP_TEST_STR_3, "xxxxstringxxxx");

    char* HEAP_TEST_STRS[] = {
        HEAP_TEST_STR_1,
        HEAP_TEST_STR_2,
        HEAP_TEST_STR_3,
        NULL,
    };

    char* const HEAP_EXPECTED[] = {
        "string",
        "stringxxxx",
        "stringxxxx",
    };

    char TEST_STRS[][20] = {
        "xxxxstringxx",
        "string",
        "",
        "xxstring",
        "stringxxxx",
    };

    char* const EXPECTED[] = {
        "stringxx",
        "string",
        "",
        "string",
        "stringxxxx",
    };

    int const N_TEST_STRS = sizeof(TEST_STRS) / 20;

    for (int i = 0; HEAP_TEST_STRS[i]; i++) {
        trimcl(HEAP_TEST_STRS[i], 'x');
        assert(strcmp(HEAP_EXPECTED[i], HEAP_TEST_STRS[i]) == 0);
    }

    for (int i = 0; i < N_TEST_STRS; i++) {
        trimcl(TEST_STRS[i], 'x');
        assert(strcmp(EXPECTED[i], TEST_STRS[i]) == 0);
    }

    free(HEAP_TEST_STR_1);
    free(HEAP_TEST_STR_2);
    free(HEAP_TEST_STR_3);    
}

void trimc_test() {

    char* HEAP_TEST_STR_1 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_2 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_3 = calloc(BUF_SIZE, sizeof(char));

    strcpy(HEAP_TEST_STR_1, "xxxxstring");
    strcpy(HEAP_TEST_STR_2, "stringxxxx");
    strcpy(HEAP_TEST_STR_3, "xxxxstringxxxx");

    char* HEAP_TEST_STRS[] = {
        HEAP_TEST_STR_1,
        HEAP_TEST_STR_2,
        HEAP_TEST_STR_3,
        NULL,
    };

    char* const HEAP_EXPECTED[] = {
        "string",
        "string",
        "string",
        NULL,
    };

    char TEST_STRS[][20] = {
        "xxxxstringxx",
        "string",
        "",
        "xxstring",
        "stringxxxx",
    };

    char* const EXPECTED[] = {
        "string",
        "string",
        "",
        "string",
        "string",
    };

    int const N_TEST_STRS = sizeof(TEST_STRS) / 20;

    for (int i = 0; HEAP_TEST_STRS[i]; i++) {
        trimc(HEAP_TEST_STRS[i], 'x');
        assert(strcmp(HEAP_EXPECTED[i], HEAP_TEST_STRS[i]) == 0);
    }

    for (int i = 0; i < N_TEST_STRS; i++) {
        trimc(TEST_STRS[i], 'x');
        assert(strcmp(EXPECTED[i], TEST_STRS[i]) == 0);
    }

    free(HEAP_TEST_STR_1);
    free(HEAP_TEST_STR_2);
    free(HEAP_TEST_STR_3);    
}

void trimr_test() {

    char* HEAP_TEST_STR_1 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_2 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_3 = calloc(BUF_SIZE, sizeof(char));

    strcpy(HEAP_TEST_STR_1, "    string");
    strcpy(HEAP_TEST_STR_2, "string    ");
    strcpy(HEAP_TEST_STR_3, "    string    ");

    char* HEAP_TEST_STRS[] = {
        HEAP_TEST_STR_1,
        HEAP_TEST_STR_2,
        HEAP_TEST_STR_3,
        NULL,
    };

    char* const HEAP_EXPECTED[] = {
        "    string",
        "string",
        "    string",
        NULL,
    };

    char TEST_STRS[][20] = {
        "    string  ",
        "string",
        "",
        "  string",
        "string    ",
    };

    char* const EXPECTED[] = {
        "    string",
        "string",
        "",
        "  string",
        "string",
    };

    int const N_TEST_STRS = sizeof(TEST_STRS) / 20;

    for (int i = 0; HEAP_TEST_STRS[i]; i++) {
        trimr(HEAP_TEST_STRS[i]);
        assert(strcmp(HEAP_EXPECTED[i], HEAP_TEST_STRS[i]) == 0);
    }

    for (int i = 0; i < N_TEST_STRS; i++) {
        trimr(TEST_STRS[i]);
        assert(strcmp(EXPECTED[i], TEST_STRS[i]) == 0);
    }

    free(HEAP_TEST_STR_1);
    free(HEAP_TEST_STR_2);
    free(HEAP_TEST_STR_3);    
}

void triml_test() {

    char* HEAP_TEST_STR_1 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_2 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_3 = calloc(BUF_SIZE, sizeof(char));

    strcpy(HEAP_TEST_STR_1, "    string");
    strcpy(HEAP_TEST_STR_2, "string    ");
    strcpy(HEAP_TEST_STR_3, "    string    ");

    char* HEAP_TEST_STRS[] = {
        HEAP_TEST_STR_1,
        HEAP_TEST_STR_2,
        HEAP_TEST_STR_3,
        NULL,
    };

    char* const HEAP_EXPECTED[] = {
        "string",
        "string    ",
        "string    ",
        NULL,
    };

    char TEST_STRS[][20] = {
        "    string  ",
        "string",
        "",
        "  string",
        "string    ",
    };

    char* const EXPECTED[] = {
        "string  ",
        "string",
        "",
        "string",
        "string    ",
    };

    int const N_TEST_STRS = sizeof(TEST_STRS) / 20;

    for (int i = 0; HEAP_TEST_STRS[i]; i++) {
        triml(HEAP_TEST_STRS[i]);
        assert(strcmp(HEAP_EXPECTED[i], HEAP_TEST_STRS[i]) == 0);
    }

    for (int i = 0; i < N_TEST_STRS; i++) {
        triml(TEST_STRS[i]);
        assert(strcmp(EXPECTED[i], TEST_STRS[i]) == 0);
    }

    free(HEAP_TEST_STR_1);
    free(HEAP_TEST_STR_2);
    free(HEAP_TEST_STR_3);    
}

void trim_test() {

    char* HEAP_TEST_STR_1 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_2 = calloc(BUF_SIZE, sizeof(char));
    char* HEAP_TEST_STR_3 = calloc(BUF_SIZE, sizeof(char));

    strcpy(HEAP_TEST_STR_1, "    string");
    strcpy(HEAP_TEST_STR_2, "string    ");
    strcpy(HEAP_TEST_STR_3, "    string    ");

    char* HEAP_TEST_STRS[] = {
        HEAP_TEST_STR_1,
        HEAP_TEST_STR_2,
        HEAP_TEST_STR_3,
        NULL,
    };

    char* const HEAP_EXPECTED[] = {
        "string",
        "string",
        "string",
        NULL,
    };

    char TEST_STRS[][20] = {
        "    string  ",
        "string",
        "",
        "  string",
        "string    ",
    };

    char* const EXPECTED[] = {
        "string",
        "string",
        "",
        "string",
        "string",
    };

    int const N_TEST_STRS = sizeof(TEST_STRS) / 20;

    for (int i = 0; HEAP_TEST_STRS[i]; i++) {
        trim(HEAP_TEST_STRS[i]);
        assert(strcmp(HEAP_EXPECTED[i], HEAP_TEST_STRS[i]) == 0);
    }

    for (int i = 0; i < N_TEST_STRS; i++) {
        trim(TEST_STRS[i]);
        assert(strcmp(EXPECTED[i], TEST_STRS[i]) == 0);
    }

    free(HEAP_TEST_STR_1);
    free(HEAP_TEST_STR_2);
    free(HEAP_TEST_STR_3);    
}
