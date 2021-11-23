#ifndef NULL
    #include <string.h>
#endif

#ifndef SODIUM_PLUS_PLUS
    #include "../headers/sodiumplusplus.h"
#endif

#ifndef STDIO_PLUS_PLUS
    #include "../headers/stdioplusplus.h"
#endif

#ifndef TERMIOS_PLUS_PLUS
    #include "../headers/termiosplusplus.h"
#endif

#include "../headers/input_acquisition.h"

/*----------CONSTANTS-DEFINITION-START----------*/

#define LINE_BUFSIZE 1024

/*-----------CONSTANTS-DEFINITION-END-----------*/

// this function securely reads all bytes from stdin using libsodium library (doc.libsodium.org).
char *read_line_s(void) 
{
    size_t buf_size = LINE_BUFSIZE;
    size_t old_size;

    char *buffer = (char *) sodium_malloc(sizeof(char) * buf_size);

    int pos = 0;
    char c = 0x00;
    struct termios old;

    if (!buffer) {
        perror("psm: allocation error\n");
        return NULL;
    }

    // disabling echo for security purposes
    old = disable_terminal_echo();

    while ((c = getchar()) != EOF && c != '\n') {
        buffer[pos] = c;
        pos++;
        // if the buffer is not large enough, it gets stretched (sodium_realloc is a custom function)
        if (pos > buf_size) {
            old_size = buf_size;
            buf_size += LINE_BUFSIZE;
            buffer = (char *) sodium_realloc(buffer, old_size, buf_size);
        }
    }

    buffer[pos] = '\0';

    // re-enabling echo
    enable_terminal_echo(old);
    
    return buffer;
}

// this function simply reads all bytes from stdin
char *read_line(void) 
{
    size_t bufsize = LINE_BUFSIZE;

    char *buffer = malloc(sizeof(char) * bufsize);
    
    int pos = 0;
    char c; 

    if (!buffer) {
        perror("psm: allocation error\n");
        return NULL;
    }

    while ((c = getchar()) != EOF && c != '\n')
    {
        buffer[pos] = c;
        pos++;

        // if the buffer is too short, it gets stretched
        if (pos >= bufsize) {
            bufsize += LINE_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                perror("psm: allocation error\n");
                return NULL;
            }
        }
    }

    buffer[pos] = '\0';
    return buffer;
}