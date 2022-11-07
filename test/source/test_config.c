#include "../../utils/headers/config.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {

    printf("test: test start\n");

    printf("test: calling functions\n");

    char *path = get_config_path();
    printf("test 1: path: %s\n", path);

    int err;

    if ((err = add_env_var("TEST", "test")) != 0) {
        printf("test 2 failed with error code: %d\n", err);
    } else {
        printf("test 2: check config.test"); 
    }

    char *val = get_env_var("TEST");
    printf("test 3: value of key \"TEST\": %s\n", val);

    free(path);
    free(val);

    printf("test: test end\n");

    return 0;
}