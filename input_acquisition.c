#ifndef NULL
    #include <string.h>
#endif

#ifndef TERMIOS_PLUS_PLUS
    #include <termiosplusplus.h>
#endif

/*----------CONSTANTS-DEFINITION-START----------*/

#define LINE_BUFSIZE 1024

/*-----------CONSTANTS-DEFINITION-END-----------*/



/*----------FUNCTIONS-DEFINITION-START----------*/

int read_pass(char *buffer, size_t max_buff_size);
char *read_line(void) ;

/*-----------FUNCTIONS-DEFINITION-END-----------*/

// this function securely get a password from stdin using libsodium library
// (doc.libsodium.org). It returns 0 for false (bad password), 1 for true
// (good password) and -1 for errors.
int read_pass(char *buffer, size_t max_buff_size) 
{
    size_t bufsize = max_buff_size;
    int position = 0;
    char c = 0x00;
    struct termios old;

    if (!buffer) {
        perror("psm: allocation error\n");
        return -1;
    }

    // disabling echo for security purposes
    old = disable_terminal_echo();

    while ((c = getchar()) != EOF && c != '\n') {
        buffer[position] = c;
        position++;
        if (position > bufsize) {
            enable_terminal_echo(old);
            printf("psm: password length cannot be larger than %d\n", bufsize);
            return 0;
        }
    }

    buffer[position] = '\0';

    // re-enabling echo
    enable_terminal_echo(old);

    return 1;
}

// this function simply reads all bytes from stdin
char *read_line(void) 
{
    size_t bufsize = LINE_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c; 

    if (!buffer) {
        perror("psm: allocation error\n");
        return NULL;
    }

    while ((c = getchar()) != EOF && c != '\n')
    {
        buffer[position] = c;
        position++;

        // if the buffer is too short, it gets stretched
        if (position >= bufsize) {
            bufsize += LINE_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                perror("psm: allocation error\n");
                return -1;
            }
        }
    }

    buffer[position] = '\0';
    return buffer;
}