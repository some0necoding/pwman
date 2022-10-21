#include <stdio.h>

int psm_help(char **args)
{
    unsigned char *help_txt;

    if (args[1]) {
        printf("\"help\" does not accept arguments\n");
        return -1;
    }

    help_txt = "\nWHAT IS PASSMAN:\n"
               "\n"
               "    Passman stores and retrieves accounts along with\n"
	           "    their passwords.\n"
               "\n"
	           "    Every account is made up by three parts:\n"
		       "        - account name\n" 
		       "        - email or username used to log in the account\n"
		       "        - password used to log in the account\n"
               "\n"
               "SYNTAX:\n" 
               "\n"
	           "    show\n" 
	           "    add <account-name> <user-or-mail>\n" 
	           "    rm <account-name>\n"
	           "    get <account-name>\n" 
	           "    help\n"
               "\n"
               "DESCRIPTION:\n"
               "\n"
	           "    show\n" 
		       "        To show all accounts stored.\n"
               "\n"
	           "    add\n" 
		       "        To add a new account.\n"
               "\n"
	           "    rm\n"
		       "        To remove an account.\n" 
               "\n"
	           "    get\n"
		       "        To retrieve a password. Note that it will be saved\n"
               "        into the (X11) clipboard so that it can be directly\n"
               "        pasted.\n"
               "\n"
	           "    help\n" 
		       "        To display help page.\n";

    printf("%s\n", help_txt);

    return 0;
}