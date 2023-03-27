#ifndef ENVIRONMENT_VARIABLES
#define ENVIRONMENT_VARIABLES

const char *get_config_path();
int psm_putenv(const char *key, const char *value); // ex add_env_var()
const char *psm_getenv(const char *key);			// ex get_env_var()

#endif
