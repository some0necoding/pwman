#ifndef STDIO_PLUS_PLUS
    #include "../headers/stdioplusplus.h"
#endif

#ifndef ECHOCTL
    #include <termios.h>
#endif

#include "../headers/termiosplusplus.h"

struct termios disable_terminal_echo() {

    struct termios old_t;
    struct termios new_t;

    (void) tcgetattr(fileno(stdin), &old_t);

    new_t = old_t;
    new_t.c_lflag &= ~ECHO;

    (void) tcsetattr(fileno(stdin), TCSAFLUSH, &new_t);

    return old_t;
}

void enable_terminal_echo(struct termios old_t) {
    (void) tcsetattr(fileno(stdin), TCSAFLUSH, &old_t);
}