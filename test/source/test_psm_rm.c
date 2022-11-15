#include "../../commands/headers/psm_rm.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {

    printf("test: test start\n");

    printf("test: calling functions\n");
    
    char **args = calloc(2, sizeof(char *));
    args[0] = "rm";
    args[1] = "test_file";

    psm_rm(args);

    free(args);

    printf("test: test end\n");

    return 0;
}