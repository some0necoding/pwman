#include "../headers/psm_help.h"

#include <stdio.h>

int psm_help(char **args)
{
    char *help_txt;

    if (args[1]) {
        printf("\"help\" does not accept arguments\n");
        return -1;
    }

    help_txt = "\n"
               "    Pwman is a really simple password manager\n"
               "\n"
               "SYNTAX:\n" 
	           "    show [FILE]\n" 
		       "        Show all stored files.\n"
	           "    add FILE\n" 
		       "        Add a new file.\n"
	           "    rm FILE\n"
		       "        Remove a file.\n" 
	           "    get FILE\n"
		       "        Retrieve a password and stores it into the clipboard.\n"
	           "    help\n" 
		       "        To display help page.\n";

    printf("%s\n", help_txt);

    return 0;
}