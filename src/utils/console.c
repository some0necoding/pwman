#include "./console.h"

#include <stdio.h>
#include <termios.h>

struct termios disable_terminal_echo() {

    struct termios old_t;
    struct termios new_t;

    tcgetattr(fileno(stdin), &old_t);

    new_t = old_t;
    new_t.c_lflag &= ~ECHO;

    tcsetattr(fileno(stdin), TCSAFLUSH, &new_t);

    return old_t;
}

void enable_terminal_echo(struct termios old_t) {
    tcsetattr(fileno(stdin), TCSAFLUSH, &old_t);
}
