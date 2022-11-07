#include "../../commands/headers/psm_show.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

int main(int argc, char const *argv[]) {

    printf("test: test start\n");

    printf("test: calling functions\n");
    
    char **args = calloc(2, sizeof(char *));
    args[0] = "show";
    args[1] = "amazon";

    psm_show(args);

    free(args);

    printf("test: test end\n");

    return 0;
}