#ifndef CONFIG_UTIL
#define CONFIG_UTIL

#include <stddef.h>
#include <stdbool.h>

#define CONFIG_LEN 2

/**
 * List of data types supported by config files
 */
enum Types {
    STRING,
};

/**
 * List of all pwman environment variables.
 */
typedef struct {
    char const *storage;    /**< Path to the password store */
    char const *gpg_fpr;    /**< Fingerprint of the gpg key used */
} Config;

/**
 * This struct is used to map every variable of @ref Config to its name.
 */
typedef struct {
    enum Types type;        /**< Data type of *var */
    bool is_required;       /**< Is a required variable */
    char* name;             /**< Variable name  */
    void **var;             /**< Pointer to a @ref Config variable */
} VariableMapping;


/* 
 * list of possible locations of the config files, ordered from the lowest
 * to the highest priority 
 */
char const* const CONFIG_FILES[] = {
    "/etc/pwman.conf",
    "$HOME/.config/pwman/pwman.conf",
    NULL,
};


/**
 * Read all config files in the opposite order of priority setting variables
 * inside @ref Config
 *
 * @return a @ref Config instance containing variables set by config files.
 */
Config* read_config(char const* const path_list[static 1]);

#endif
