#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../src/utils/dynstr.h"

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
    char const* PREFIX = "prefix";
    char const* SUFFIX = "suffix";
    char* test_str_nempty = (char*) calloc(strlen(PREFIX) + strlen(SUFFIX) + 1, sizeof(char));
    char* test_str_empty = (char*) calloc(strlen(SUFFIX) + 1, sizeof(char));

    // test all possible lengths
    for (int i = 0; i <= strlen(SUFFIX); i++) {

        // test with prefix
        snprintf(test_str_nempty, strlen(PREFIX) + i + 1, "%s%s", PREFIX, SUFFIX);
        dynstr* d_nempty = dynstr_news(PREFIX);
        dynstr_appendn(d_nempty, SUFFIX, i);

        assert(strcmp(d_nempty->s, test_str_nempty) == 0);

        // test without prefix
        strncpy(test_str_empty, SUFFIX, i);
        dynstr* d_empty = dynstr_new();
        dynstr_appendn(d_empty, SUFFIX, i);

        assert(strcmp(d_empty->s, test_str_empty) == 0);
    }
}

void dynstr_append_test() {
    char const* PREFIX = "prefix";
    char const* SUFFIX = "suffix";
    char* test_str_nempty = (char*) calloc(strlen(PREFIX) + strlen(SUFFIX) + 1, sizeof(char));
    char* test_str_empty = (char*) calloc(strlen(SUFFIX) + 1, sizeof(char));

    // test with prefix
    snprintf(test_str_nempty, strlen(PREFIX) + strlen(SUFFIX) + 1, "%s%s", PREFIX, SUFFIX);
    dynstr* d_nempty = dynstr_news(PREFIX);
    dynstr_append(d_nempty, SUFFIX);

    assert(strcmp(d_nempty->s, test_str_nempty) == 0);

    // test without prefix
    strcpy(test_str_empty, SUFFIX);
    dynstr* d_empty = dynstr_new();
    dynstr_append(d_empty, SUFFIX);

    assert(strcmp(d_empty->s, test_str_empty) == 0);
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
