#include "../headers/termiosplusplus.h"

#include "../headers/stdioplusplus.h"

#include <termios.h>

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