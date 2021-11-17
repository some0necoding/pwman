#ifndef ECHOCTL
    #include <termios.h>
#endif

#ifndef TERMIOS_PLUS_PLUS
#define TERMIOS_PLUS_PLUS

// disables echo on terminal and returns old configs with echo enabled
struct termios disable_terminal_echo();

// reapply old configs returned before to re-enable echo on terminal
void enable_terminal_echo(struct termios old_t);

#endif