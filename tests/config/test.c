#include <stdio.h>
#include "../../src/config/config.h"

#define TEST_DEFAULT "${XDG_CONFIG_HOME}/pwman/and/some/other/directory/that/puts/this/program/in/trouble/pwman.conf"

int main() {
    char const* s = expand_env(TEST_DEFAULT);
    if (!s) 
        return -1;
    printf("%s\n", s);
    return 0;
}
