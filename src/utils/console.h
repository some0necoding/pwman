#ifndef TERMIOS_PLUS_PLUS
#define TERMIOS_PLUS_PLUS

#include <termios.h>

// disables echo on terminal and returns old configs with echo enabled
const struct termios disable_terminal_echo();

// reapply old configs returned before to re-enable echo on terminal
void enable_terminal_echo(const struct termios old_t);

#endif
