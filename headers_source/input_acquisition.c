#include "../headers/input_acquisition.h"

#include "../headers/sodiumplusplus.h"
#include "../headers/stdioplusplus.h"
#include "../headers/termiosplusplus.h"

#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define LINE_BUFSIZE 1024

/*-----------CONSTANTS-DEFINITION-END-----------*/

// this function securely (i.e. disabling echo) reads all bytes from stdin and 
// stores them into "buffer" (that needs to be allocated using sodium_malloc).
int read_line_s(char **buffer, size_t bufsize) 
{
    size_t tmp_size = LINE_BUFSIZE;
    size_t old_size;

    char *tmp_buff = (char *) sodium_malloc(tmp_size);

    char c = 0x00;
    struct termios old;

    int pos = 0;
    int ret_code = -1;

    if (!tmp_buff) {
        printf("psm: allocation error\n");
        return -1;
    }

    old = disable_terminal_echo();                              // disabling echo

    while ((c = getchar()) != EOF && c != '\n') 
    {
        tmp_buff[pos] = c;
        pos++;

        // if the buffer is not large enough, it gets stretched (sodium_realloc is a custom function)
        if (pos > tmp_size) {
            old_size = tmp_size;
            tmp_size += LINE_BUFSIZE;
            tmp_buff = (char *) sodium_realloc(tmp_buff, old_size, tmp_size);
            if (!tmp_buff) {
                perror("psm: allocation error");
                goto ret;
            }
        }
    }

    tmp_buff[pos] = '\0';

    if (bufsize < (pos + 1)) {
        *buffer = (char *) sodium_realloc(*buffer, bufsize, (pos + 1));

        if (!*buffer) {
            perror("psm: allocation error\n");
            goto ret;
        }
    }

    memcpy(*buffer, tmp_buff, (pos + 1));

    ret_code = 0;

ret: 
    sodium_free(tmp_buff);
    enable_terminal_echo(old);                                  // re-enabling echo
    return ret_code;
}

// this function simply reads all bytes from stdin and stores them into
// "buffer" (that needs to be allocated using malloc).
int read_line(char **buffer, size_t bufsize) 
{    
    size_t tmp_size = LINE_BUFSIZE;

    char *tmp_buff = (char *) malloc(tmp_size);

    char c;

    int pos = 0;
    int ret_code = -1;

    if (!tmp_buff) {
        printf("psm: allocation error\n");
        return -1;
    }

    while ((c = getchar()) != EOF && c != '\n')
    {
        tmp_buff[pos] = c;
        pos++;

        // if the buffer is too short, it gets stretched
        if (pos >= tmp_size) {
            tmp_size += LINE_BUFSIZE;
            tmp_buff = (char *) realloc(tmp_buff, tmp_size);
            if (!tmp_buff) {
                perror("psm: allocation error");
                goto ret;
            }
        }
    }

    tmp_buff[pos] = '\0';

    if (bufsize < (pos + 1)) {
        *buffer = (char *) realloc(*buffer, pos);

        if (!*buffer) {
            printf("psm: allocation error\n");
            goto ret;
        }
    }

    memcpy(*buffer, tmp_buff, (pos + 1));

    ret_code = 0;

ret:
    free(tmp_buff);
    return ret_code;
}