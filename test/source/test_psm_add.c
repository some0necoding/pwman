#include "../../commands/headers/psm_add.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {

    printf("test: test start\n");

    printf("test: calling functions\n");
    
    char **args = calloc(2, sizeof(char *));
    args[0] = "add";
    //args[1] = "test_file";

    psm_add(args);

    free(args);

    printf("test: test end\n");

    return 0;
}