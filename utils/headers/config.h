#ifndef ENVIRONMENT_VARIABLES
#define ENVIRONMENT_VARIABLES

#define CONFIG_FILE "/home/marco/.config/pwman.conf"

char *get_config_path();
int add_env_var(char *key, char *value);
char *get_env_var(char *key);

#endif