#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../src/config/config.c"


#define BUF_SIZE 64


void Config_init_test();
void Config_free_test();
void VariableMapping_init_test();
void VariableMapping_set_test();
void VariableMapping_defaults_test();
void VariableMapping_free_test();
void expand_env_test();
void parse_config_file_test();
void split_name_value_test();
void remove_comment_test();
void read_config_test();

static VariableMapping* VariableMapping_search(VariableMapping** v, char const name[static 1]);

static void VariableMapping_init_var_test(VariableMapping* v[static 1], 
        char const name[static 1], void** config_var, bool is_required, enum Types type);

static int ignore_stderr();
static void reset_stderr(int old_stderr);

int main() {

    printf("== RUN -- expand_env_test\n");
    expand_env_test();
    printf("== PASS\n");

    printf("== RUN -- Config_init_test\n");
    Config_init_test();
    printf("== PASS\n");

    printf("== RUN -- Config_free_test\n");
    Config_free_test();
    printf("== PASS\n");

    printf("== RUN -- VariableMapping_init_test\n");
    VariableMapping_init_test();
    printf("== PASS\n");

    printf("== RUN -- VariableMapping_set_test\n");
    VariableMapping_set_test();
    printf("== PASS\n");

    printf("== RUN -- VariableMapping_defaults_test\n");
    VariableMapping_defaults_test();
    printf("== PASS\n");

    printf("== RUN -- split_name_value_test\n");
    split_name_value_test();
    printf("== PASS\n");

    printf("== RUN -- remove_comment_test\n");
    remove_comment_test();
    printf("== PASS\n");

    printf("== RUN -- parse_config_file_test\n");
    parse_config_file_test();
    printf("== PASS\n");

    printf("== RUN -- read_config_test\n");
    read_config_test();
    printf("== PASS\n");

    return 0;
}

void expand_env_test() {

    char* const TEST_STRS[] = {
        "${XDG_CONFIG_HOME}/some/directory",
        "${HOME}/.config",
        "/some/dir${XDG_CONFIG_HOME}/and/some/other",
        "${UNEXISTENT_VARIABLE}/.config",
        "/this/dir/ends/with${HOME}",
        NULL,
    };

    char* const EXPECTED[] = {
         "/home/marco/.config/some/directory" ,
         "/home/marco/.config" ,
         "/some/dir/home/marco/.config/and/some/other" ,
        NULL,
         "/this/dir/ends/with/home/marco" ,
        NULL,
    };

    for (int i = 0; TEST_STRS[i]; i++) {
        int old_stderr = ignore_stderr();
        char* s = expand_env(TEST_STRS[i]);
        reset_stderr(old_stderr);

        if (!s) assert(!EXPECTED[i]);
        else    assert(strcmp(s, EXPECTED[i]) == 0);
    }
}

void Config_init_test() {
    Config* c = Config_init();
    if (!c) return;

    assert(!c->storage);
    assert(!c->gpg_fpr);
}

void Config_free_test() {
    Config* c = Config_init();
    if (!c) return;
    Config_free(c);
}

void VariableMapping_init_test() {
    Config* c = Config_init();
    if (!c) return;
    VariableMapping** v = VariableMapping_init(c);
    if (!v) return;
    VariableMapping_init_var_test(v, "storage", (void**) &c->storage, true, STRING);
    VariableMapping_init_var_test(v, "gpg_key", (void**) &c->gpg_fpr, true, STRING);
    VariableMapping_free(v);
    Config_free(c);
}

static void VariableMapping_init_var_test(VariableMapping* v[static 1], 
        char const name[static 1], void** config_var, bool is_required, enum Types type) {
    VariableMapping* storage = VariableMapping_search(v, name);
    assert(storage->var == (void**) config_var);
    assert(storage->is_required == is_required);
    assert(storage->type == type);
    
}

void VariableMapping_set_test() {

    char* const TEST_STRS[] = {
        "something else",
        "string",
        "random long string",
        "${HOME}/tricky/path",
        "${UNEXISTENT_VARIABLE}/tricky/path",
        NULL,
    };

    char* const EXPECTED[] = {
        "something else",
        "string",
        "random long string",
        "/home/marco/tricky/path",
        "/home/marco/tricky/path", // remains unchanged
        NULL,
    };

    Config* c = Config_init();
    if (!c) return;
    VariableMapping** v = VariableMapping_init(c);
    if (!v) return;

    for (int i = 0; TEST_STRS[i]; i++) {
        int old_stderr = ignore_stderr();
        assert(!VariableMapping_set(v, "storage", TEST_STRS[i]));
        assert(strcmp(c->storage, EXPECTED[i]) == 0);
        reset_stderr(old_stderr);
    }

    // test wrong names
    int old_stderr = ignore_stderr();
    assert(VariableMapping_set(v, "wrong name", TEST_STRS[0]));
    assert(VariableMapping_set(v, "anotherwrongname", TEST_STRS[0]));
    reset_stderr(old_stderr);
}

void VariableMapping_defaults_test() {
    Config* c = Config_init();
    if (!c) return;
    VariableMapping** m = VariableMapping_init(c);
    if (!m) return;
    VariableMapping_defaults(m);

    assert(strcmp(c->storage, expand_env("${HOME}/.pwstore")) == 0);
    assert(!c->gpg_fpr);
}

void split_name_value_test() {

    char* const TEST_STRS[] = {
        "name=value",
        "name = value",
        "name   =   value",
        "name with spaces = value with spaces",
        "name = value with = sign inside",
        "name with = sign inside = value",
        "line without equal sign",
        NULL,
    };

    char* const EXPECTED[][2] = {
        { "name", "value" },
        { "name", "value" },
        { "name", "value" },
        { "name with spaces", "value with spaces" },
        { "name" , "value with = sign inside" },
        { "name with", "sign inside = value" },
        NULL,
        NULL,
    };

    char* s;
    for (int i = 0; (s = TEST_STRS[i]); i++) {
        char** result = split_name_value(s, strlen(s));
        if (!result)
            assert(!EXPECTED[i][0] && !EXPECTED[i][0]);
        else {
            assert(strcmp(result[0], EXPECTED[i][0]) == 0);
            assert(strcmp(result[1], EXPECTED[i][1]) == 0);
            free(result[0]);
            free(result[1]);
            free(result);
        }
    }
}

void remove_comment_test() {

    char TEST_STRS[][50] = {
        "line ending with empty comment #",
        "line ending with comment # this is a comment",
        "# line starting with comment",
        "# line with # nested comments",
    };

    int N_TEST_STRS = sizeof(TEST_STRS) / 50;

    char* const EXPECTED[] = {
        "line ending with empty comment",
        "line ending with comment",
        "",
        "",
    };

    for (int i = 0; i < N_TEST_STRS; i++) {
        remove_comment(TEST_STRS[i]);
        assert(strcmp(TEST_STRS[i], EXPECTED[i]) == 0);
    }
}

void parse_config_file_test() {
    Config* c = Config_init();
    if (!c) return;
    VariableMapping** m = VariableMapping_init(c);
    if (!m) return;
    VariableMapping_defaults(m);

    int old_stderr = ignore_stderr();
    parse_config_file("tests/config/pwman.conf.test.1", m);
    reset_stderr(old_stderr);

    assert(strcmp(c->storage, expand_env("${HOME}/somerandomfolder")) == 0);
    assert(strcmp(c->gpg_fpr, "K235J4LKJRH2FXM3R9F8N") == 0);

    old_stderr = ignore_stderr();
    parse_config_file("tests/config/pwman.conf.test.2", m);
    reset_stderr(old_stderr);

    assert(strcmp(c->storage, expand_env("${XDG_CONFIG_HOME}/some/other/random/folder")) == 0);
    assert(strcmp(c->gpg_fpr, "LNxzlWeKlktbSGKJHsW6") == 0);

    old_stderr = ignore_stderr();
    parse_config_file("tests/config/pwman.conf.test.3", m);
    reset_stderr(old_stderr);

    assert(strcmp(c->storage, expand_env("${XDG_CONFIG_HOME}/some/other/random/folder")) == 0);
    assert(strcmp(c->gpg_fpr, "LNxzlWeKlktbSGKJHsW6") == 0);
}

void read_config_test() {

    char const* const TEST_CONFIG_FILES[] = {
        "tests/config/pwman.conf.test.1",
        "tests/config/pwman.conf.test.2",
        "tests/config/pwman.conf.test.3",
        NULL,
    };

    int old_stderr = ignore_stderr();
    Config* c = read_config(TEST_CONFIG_FILES);
    reset_stderr(old_stderr);

    assert(strcmp(c->storage, expand_env("${XDG_CONFIG_HOME}/some/other/random/folder")) == 0);
    assert(strcmp(c->gpg_fpr, "LNxzlWeKlktbSGKJHsW6") == 0);
}

// ------------- TEST UTILITY FUNCTIONS ------------- //

/**
 * Search for the variable associated to name inside v. If v does not contain 
 * name NULL is returned.
 */
static VariableMapping* VariableMapping_search(VariableMapping** v, char const name[static 1]) {
    for (int i = 0; v[i]; i++)
        if (!strcmp(v[i]->name, name))
            return v[i];
    return NULL;
}

static int ignore_stderr() {
    int old_stderr = dup(STDERR_FILENO);
    if (old_stderr < 0)
        return -1;
    FILE* devnull = fopen("/dev/null", "r");
    if (!devnull)
        return -1;
    if (dup2(fileno(devnull), STDERR_FILENO) == -1) {
        printf("ignore_stderr: errno: %d\n", errno);
        return -1;
    }
    return old_stderr;
}

static void reset_stderr(int old_stderr) {
    dup2(old_stderr, STDERR_FILENO);
}
