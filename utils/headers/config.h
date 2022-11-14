#ifndef ENVIRONMENT_VARIABLES
#define ENVIRONMENT_VARIABLES

char *get_config_path();
int add_env_var(char *key, char *value);
char *get_env_var(char *key);

#endif