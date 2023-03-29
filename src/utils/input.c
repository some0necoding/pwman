#include "./input.h"
#include "./console.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define MAX_LINE_LENGTH 4096
#define BUFSIZE_S 32
#define BUFSIZE 1024


/*
    This function reads all bytes from stdin
    disabling echo.
*/
char *read_line_s() 
{
    struct termios old;

	char *line = (char *) malloc(sizeof(char) * (MAX_LINE_LENGTH));	
    char *ret_val = NULL;

    char c;
   
	int pos = 0;

	if (!line) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		goto ret;
	}

    old = disable_terminal_echo();

    while (((c = getchar()) != EOF) && (c != '\n') && (pos < MAX_LINE_LENGTH)) {
		line[pos++] = c;
    }

	if (c == EOF && ferror(stdin)) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		goto ret;
	}

	line[pos++] = '\0';
    ret_val = line;

ret:
    enable_terminal_echo(old);
    return ret_val;
}


/*
    This function reads all bytes from stdin.
*/
char *read_line() 
{
	char *line = (char *) malloc(sizeof(char) * MAX_LINE_LENGTH);

	char c;
    
	int pos = 0;

	if (!line) {
		fprintf(stderr, "psm:%s:%d: allocation error\n", __FILE__, __LINE__);
		return NULL;
	}

    while (((c = getchar()) != EOF) && (c != '\n') && (pos < MAX_LINE_LENGTH)) {
		line[pos++] = c;
    }

	if (c == EOF && ferror(stdin)) {
		fprintf(stderr, "psm:%s:%d: I/O error\n", __FILE__, __LINE__);
		return NULL;
	}
	
	line[pos++] = '\0';

    return line;
}
