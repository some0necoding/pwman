#ifndef INPUT_ACQUISITION
#define INPUT_ACQUISITION

// securely reads all bytes from stdin (good for passwords and logins)
extern char *read_line_s(void);

// reads all bytes from stdin
extern char *read_line(void);

#endif