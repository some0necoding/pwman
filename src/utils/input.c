#include "./input.h"
#include "./console.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*----------CONSTANTS-DEFINITION-START----------*/

#define BUFSIZE_S 32
#define BUFSIZE 1024

/*-----------CONSTANTS-DEFINITION-END-----------*/

/*
    This function reads all bytes from stdin
    disabling echo and stores them into a buffer.
*/
int read_line_s(char **buffer, size_t bufsize) 
{
    struct termios old;

    char ch;
    int pos = 0;
    int ret_code = -1;

    /* Disable echo */
    old = disable_terminal_echo();

    while ((ch = getchar()) != EOF && ch != '\n') {
        
        memcpy(*buffer+pos, &ch, sizeof(char));
        pos++;

        /* Stretch buffer if needed */
        if (pos >= bufsize) {
            
            bufsize += BUFSIZE_S;
            *buffer = realloc(*buffer, sizeof(char *) * bufsize);
            
            if (!*buffer) {
                perror("psm: allocation error");
                goto ret;
            }
        }
    }

    memcpy(*buffer+pos, "\0", sizeof(char));
    ret_code = 0;

ret:
    /* Re-enable echo */ 
    enable_terminal_echo(old);
    return ret_code;
}

/*
    This function reads all bytes from stdin
    and stores them into a buffer.
*/
int read_line(char **buffer, size_t bufsize) 
{    
    char ch;
    int pos = 0;

    while ((ch = getchar()) != EOF && ch != '\n') {
        
        memcpy(*buffer+pos, &ch, sizeof(char));
        pos++;

        /* Stretch buffer if needed */
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
