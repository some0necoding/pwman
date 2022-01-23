#include "../headers/array_handling.h"

#include "../headers/stdioplusplus.h"
#include "../headers/sodiumplusplus.h"

int arrlen(void **arr)
{
    int size = 0;
    int pos = 0;

    while (arr[pos++] != NULL) {
        size++;
    }

    return size;
}