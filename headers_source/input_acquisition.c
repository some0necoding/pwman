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
    size_t old_size;

    char c = 0x00;
    struct termios old;

    int pos = 0;
    int ret_code = -1;

    old = disable_terminal_echo();                              // disabling echo

    while ((c = getchar()) != EOF && c != '\n') 
    {
        memcpy(*buffer+pos, &c, 1);
        pos++;

        // if the buffer is not large enough, it gets stretched (sodium_realloc is a custom function)
        if (pos >= bufsize) {
            old_size = bufsize;
            bufsize += LINE_BUFSIZE;
            *buffer = (char *) sodium_realloc(*buffer, old_size, bufsize);
            if (!*buffer) {
                perror("psm: allocation error");
                goto ret;
            }
        }
    }

    memcpy(*buffer+pos, "\0", 1);

    ret_code = 0;

ret:
    enable_terminal_echo(old);                                  // re-enabling echo
    return ret_code;
}

// this function simply reads all bytes from stdin and stores them into
// "buffer" (that needs to be allocated using malloc).
int read_line(char **buffer, size_t bufsize) 
{    
    char c;

    int pos = 0;
    int ret_code = -1;

    while ((c = getchar()) != EOF && c != '\n')
    {
        memcpy(*buffer+pos, &c, 1);
        pos++;

        // if the buffer is too short, it gets stretched
        if (pos >= bufsize) {
            bufsize += LINE_BUFSIZE;
            *buffer = (char *) realloc(*buffer, bufsize);
            if (!*buffer) {
                perror("psm: allocation error");
                goto ret;
            }
        }
    }

    memcpy(*buffer+pos, "\0", 1);

    ret_code = 0;

ret:
    return ret_code;
}