#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include "../../src/utils/dynstr.h"

void dynstr_new_test();
void dynstr_news_test();
void dynstr_appendn_test();
void dynstr_append_test();
void dynstr_tostr_test();
void dynstr_free_test();

int main() {

    printf("== RUN -- dynstr_new_test\n");
    dynstr_new_test();
    printf("== PASS\n");

    printf("== RUN -- dynstr_news_test\n");
    dynstr_news_test();
    printf("== PASS\n");

    printf("== RUN -- dynstr_appendn_test\n");
    dynstr_appendn_test();
    printf("== PASS\n");

    printf("== RUN -- dynstr_append_test\n");
    dynstr_append_test();
    printf("== PASS\n");

    printf("== RUN -- dynstr_tostr_test\n");
    dynstr_tostr_test();
    printf("== PASS\n");

    printf("== RUN -- dynstr_free_test\n");
    dynstr_free_test();
    printf("== PASS\n");

    return 0;
}

void dynstr_new_test() {
    dynstr* d = dynstr_new();
    assert(d != NULL);
    assert(d->s != NULL);
    assert(strcmp(d->s, "") == 0);
    assert(d->header.__capacity == DEFAULT_CAP);
    assert(d->header.__length == 0);
    assert(d->header.__end == d->s);
}

void dynstr_news_test() {
    const char* TEST_STR = "teststring";
    size_t testlen = strlen(TEST_STR);
    dynstr* d = dynstr_news(TEST_STR);
    assert(d != NULL);
    assert(strcmp(d->s, TEST_STR) == 0);
    assert(d->header.__capacity == testlen * 2);
    assert(d->header.__length == testlen);
    assert(d->header.__end == d->s + testlen);
}

void dynstr_appendn_test() {
    // strings to append
    char const* const TEST_STRS[] = {
        "suffix",
        "dontknow",
        "more difficult",
        "that's hard bro",
        "another one just to make sure",
    };
    size_t const NTEST_STRS = sizeof(TEST_STRS) / sizeof(TEST_STRS[0]);
    size_t TOTAL_LEN = 0;
    for (int i = 0; i < NTEST_STRS; i++)
        TOTAL_LEN += strlen(TEST_STRS[i]);

    // length of the longest test string
    size_t MAX_LEN = 0;
    for (int i = 0; i < NTEST_STRS; i++)
        MAX_LEN = MAX(strlen(TEST_STRS[i]), MAX_LEN);

    // test all possible lengths
    for (int i = 0; i <= MAX_LEN; i++) {
        char* test_str = (char*) calloc(TOTAL_LEN + 1, sizeof(char));
        for (int j = 0; j < NTEST_STRS; j++)
            test_str = strncat(test_str, TEST_STRS[j], i);

        dynstr* d = dynstr_new();
        for (int j = 0; j < NTEST_STRS; j++)
            dynstr_appendn(d, TEST_STRS[j], i);

        assert(strcmp(d->s, test_str) == 0);
    }
}

void dynstr_append_test() {
    // strings to append
    char const* const TEST_STRS[] = {
        "suffix",
        "dontknow",
        "more difficult",
        "that's hard bro",
        "another one just to make sure",
    };
    size_t const NTEST_STRS = sizeof(TEST_STRS) / sizeof(TEST_STRS[0]);
    size_t TOTAL_LEN = 0;
    for (int i = 0; i < NTEST_STRS; i++)
        TOTAL_LEN += strlen(TEST_STRS[i]);

    char* test_str = (char*) calloc(TOTAL_LEN + 1, sizeof(char));
    for (int j = 0; j < NTEST_STRS; j++)
        test_str = strcat(test_str, TEST_STRS[j]);

    dynstr* d_empty = dynstr_new();
    for (int i = 0; i < NTEST_STRS; i++)
        dynstr_append(d_empty, TEST_STRS[i]);

    assert(strcmp(d_empty->s, test_str) == 0);
}

void dynstr_tostr_test() {
    char const* TEST_STR = "some string with space and some */)@ other characters";
    dynstr* d = dynstr_news(TEST_STR);
    assert(strcmp(dynstr_tostr(d), TEST_STR) == 0);
}

void dynstr_free_test() {
    char const* TEST_STR = "some string";
    dynstr* d1 = dynstr_new();
    dynstr* d2 = dynstr_news(TEST_STR);
    dynstr_free(d1);
    dynstr_free(d2);
}
