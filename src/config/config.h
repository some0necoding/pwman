/**
 * The content of this file aims to provide functions and objects to transform 
 * a list of configuration files containing lines of the format
 *
 *      name = value
 *
 * into a struct object usable inside the application.
 *
 * CONFIGURATION FILES FORMAT
 *   All configuration files parsed should be formatted (abusing the regex 
 *   notation) in the following way:
 *
 *      \s*name\s*=\s*value\s
 *
 *   Optionally lines can contain comments starting with a '#' symbol both at 
 *   the beginning or in the middle of the line. Everything after that symbol
 *   will be removed from the parsed line, including the symbol itself and all
 *   whitespace characters before. For example:
 *
 *     'name = value    # this is a comment' -> 'name = value'
 *
 *   Any line that does not follow this format or that attempts to assign a non
 *   existent variable will be ignored.
 *   Following is a list of supported variables and their purpose:
 *      
 *     storage:     [STRING]
 *      location of the directory under which all passwords are stored. It can
 *      optionally contain environment variables enclosed between '${' and '}'.
 *     gpg_key:     [STRING]
 *      fingerprint of the gpg key used to encrypt all passwords.
 *
 * CONFIGURATION FILES LIST
 *  The global constant CONFIG_FILES contains all configuration files that will
 *  be parsed.
 *
 * STRUCT Config 
 *  After the parsing, all variables are stored inside an instance of the @ref 
 *  Config struct, which will then be used to access them by the application
 *  code. 
 *
 *   ! The number of variables contained inside the Config struct MUST be
 *  updated in the CONFIG_LEN constant.
 *   ! All variables of @ref Config MUST be initialized to NULL by the 
 *  Config_init() function (defined in config.c) and freed by the Config_free() 
 *  function (if heap allocated).
 *
 *  Methods:
 *   - Config_init()
 *   - Config_free()
 * 
 * STRUCT VariableMapping 
 *  Internally the code must be able to map @ref Config variables to their 
 *  respective names that appear inside the files. To map every variable to its
 *  name a null-terminated array of @ref VariableMapping* objects is used (defined 
 *  in config.c).
 *
 *      VariableMapping** mapping; <- array of VariableMapping* objects
 *
 *   ! All the elements of the array MUST be initialized inside the 
 *  VariableMapping_init() function by calling VariableMapping_init_var().
 *   ! All variables that have a default value should be set inside the 
 *  VariableMapping_defaults() function by calling VariableMapping_set().
 *
 *  Methods:
 *   - VariableMapping_init()
 *      - VariableMapping_init_var()
 *   - VariableMapping_set()
 *      - VariableMapping_set_string()
 *   - VariableMapping_defaults()
 *   - VariableMapping_free()
 */

#ifndef CONFIG_UTIL
#define CONFIG_UTIL

#include <stddef.h>


/**
 * List of all pwman environment variables.
 */
typedef struct {
    char const *storage;    /**< Path to the password store */
    char const *gpg_fpr;    /**< Fingerprint of the gpg key used */
} Config;

/** Number of variables contained in a @ref Config instance */
#define CONFIG_LEN 2


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
 * Read all config files contained inside @p path_list and set variables inside 
 * a @ref Config config instance
 *
 * @param   path_list a null-terminated array containing paths of config files
 * @return a @ref Config instance containing variables set by config files.
 */
Config* read_config(char const* const path_list[static 1]);


/**
 * Free a @ref Config instance.
 *
 * @param config pointer to the @ref Config variable to free
 */
void Config_free(Config config[static 1]);


#endif
