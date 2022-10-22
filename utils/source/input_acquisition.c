#include "../headers/input_acquisition.h"

#include "../headers/sodiumplusplus.h"
#include "../headers/termiosplusplus.h"

#include <stdio.h>
#include <string.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUFSIZE 1024

/*-----------CONSTANTS-DEFINITION-END-----------*/

// this function securely reads (i.e. disabling echo) all bytes from stdin and
// stores them into "buffer" (that needs to be allocated using sodium_malloc) 
int read_line_s(char **buffer, size_t bufsize) 
{
    size_t old_size;                // actual buffer size

    struct termios old;

    char c;
    int pos = 0;                    // index of the buffer
    int ret_code = -1;              // return values

    // disabling echo
    old = disable_terminal_echo();

    while ((c = getchar()) != EOF && c != '\n') 
    {
        memcpy(*buffer+pos, &c, sizeof(char));
        pos++;

        // if the buffer is not large enough, it gets stretched (sodium_realloc is a custom function)
        if (pos >= bufsize) {
            old_size = bufsize;
            bufsize += BUFSIZE;
            *buffer = (char *) sodium_realloc(*buffer, old_size, bufsize);
            if (!*buffer) {
                perror("psm: allocation error");
                goto ret;
            }
        }
    }

    memcpy(*buffer+pos, "\0", sizeof(char));
    ret_code = 0;

ret:
    // re-enabling echo
    enable_terminal_echo(old);
    return ret_code;
}

// this function simply reads all bytes from stdin and stores them into
// "buffer" (that needs to be allocated using malloc).
int read_line(char **buffer, size_t bufsize) 
{    
    char c;
    int pos = 0;                    // index of the buffer

    while ((c = getchar()) != EOF && c != '\n')
    {
        memcpy(*buffer+pos, &c, sizeof(char));
        pos++;

        // if the buffer is too short, it gets stretched
        if (pos >= bufsize) {
            bufsize += BUFSIZE;
            *buffer = (char *) realloc(*buffer, bufsize);
            if (!*buffer) {
                perror("psm: allocation error");
                return -1;
            }
        }
    }

    memcpy(*buffer+pos, "\0", sizeof(char));
    return 0;
}