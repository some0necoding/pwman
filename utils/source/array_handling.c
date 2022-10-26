#include "../headers/array_handling.h"

#include <stdio.h>

int arrlen(void **arr)
{
    int size = 0;
    int pos = 0;

    while (arr[pos++] != NULL) {
        size++;
    }

    return size;
}

int check_allocation(void **arr) 
{
    if (!*arr) {
        perror("psm: allocation error\n");
        return -1;
    }

    return 0;
}