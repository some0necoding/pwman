#ifndef ENVIRONMENT_VARIABLES
#define ENVIRONMENT_VARIABLES

#define CONFIGS "/home/marco/.config/pwman.conf" //"/etc/pwman.conf"

char *get_config_path();
int add_env_var(char *key, char *value);
char *get_env_var(char *key);

#endif