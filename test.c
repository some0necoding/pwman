#include <stdio.h>
#include <stdlib.h>

#include "./headers/stdioplusplus.h"

#define INT_LEN sizeof(unsigned int)

int main(int argc, char const *argv[])
{
    unsigned char *text1 = "abcdefghijklmnopqrstuvwxyz";
    unsigned char *text2 = (unsigned char *) malloc(30);
    char *file_path = "./file.test";

    FILE *file1 = fopen(file_path, "wb");
    fwrite(text1, 1, 26, file1);
    fclose(file1);

    FILE *file2 = fopen(file_path, "rb");
    fread(text2, 1, 4, file2);
    fclose(file2);

    text2 = fgetfromtos(file_path, 4, 6);

    text2[4] = '\0';

    printf("%s\n", text2);

    return 0;
}