#include "../utils/fio.h"
#include "./config.h"
#include "../utils/dynstr.h"
#include "../utils/estring.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <pcre.h>


/**
 * List of data types supported by config files
 */
enum Types {
    STRING,
};


/**
 * This struct is used to map every variable of @ref Config to its name.
 */
typedef struct {
    enum Types  type;           /**< Data type of var */
    bool        is_required;    /**< Is a required variable */
    char*       name;           /**< Variable name  */
    void**      var;            /**< Pointer to a @ref Config variable */
} VariableMapping;


/**
 * Initialize a @ref Config object with all default values. If a variable is
 * set to NULL it means that there is no default.
 *
 * @return a @ref Config instance containing defaults
 */
static Config* Config_init();


/**
 * Initialize an array of @ref VariableMapping instances where every element is 
 * a mapping between a @p config variable and its corresponding name.
 *
 * @param config    a @ref Config instance initialized with @ref Config_init
 * @return an array of @ref VariableMapping that maps all variables of @ref
 *         Config to their names.
 */
static VariableMapping** VariableMapping_init(Config* config);


/**
 * This function allocates @p *v and associates a reference of a variable 
 * contained inside a @ref Config instance with its @p name
 *
 * @param v             a pointer to a pointer of a @ref VariableMapping
 * @param name          the name of the variable that will be mapped
 * @param config_ref    a pointer to a pointer to a variable contained inside a 
 *                      @ref Config instance
 */
static void VariableMapping_init_var(VariableMapping* v[static 1], enum Types type,
        char const name[static 1], void* config_ref[static 1], bool is_required);


/**
 * Set the variable of name @p name with @p value inside @p mapping. It is 
 * user's responsibility to make sure that the type (@ref Types reference) 
 * associated to the VariableMapping instance of name @p name is the same as @p 
 * value.
 *
 * @param mapping   array initialized with @ref VariableMapping_init
 * @param name      name of the variable to set
 * @param value     value of @p name
 * @return 0 on success, -1 if @p name does not exist
 */
static int VariableMapping_set(VariableMapping* mapping[static 1], 
        char const name[static 1], void* value);


/**
 * Set the variable of @p v with @p value, expanding eventuals environment 
 * variables enclosed inside "${" and "}". It is user's responsibility to make
 * sure that @p v 's type is actually STRING. If value contains unexistent 
 * environment variables it is ignored and @p v remains unchanged.
 *
 * @param v         pointer to a @ref VariableMapping instance
 * @param value     null-terminated string to assign to v's mapped variable
 * @returns 0 on success, -1 if @p value contains unexistent environment
 *          variables.
 */
static int VariableMapping_set_string(VariableMapping v[static 1], 
        char value[static 1]);


/**
 * Free an array of @ref VariableMapping instances allocated by 
 * VariableMapping_init().
 *
 * @param mapping an array of pointers to @ref VariableMapping instances.
 */
static void VariableMapping_free(VariableMapping* mapping[static 1]);


/**
 * Creates a copy of s with all environment variables (e.g. ${something})
 * expanded.
 *
 * @param s     a null-terminated string optionally containing environment 
 *              variables enclosed in curly braces
 * @returns     the expanded version of s or NULL if s contains an unexistent
 *              variable
 */
static char* expand_env(char const s[static 1]);


/**
 * This function parses the content of @p file_name setting the variables 
 * inside @p mapping. If a variable is already set it is overwritten. In case
 * of any error while reading a line (non-existent variable, bad formatting, 
 * etc.) or if the line starts with a '#' that line is ignored.
 *
 * @param file_name     the name of the file to parse
 * @param mapping       the array of @ref VariableMapping instances containing 
 *                      the variables
 * @return 0 on success, -1 if @p file_name cannot be opened
 */
static int parse_config_file(char const file_name[static 1],
        VariableMapping* mapping[static 1]);


/**
 * Split a line at the first occurence of an '='. The parts before and after 
 * the '=' (excluded) are returned as null-terminated, heap allocated, 
 * whitespace-trimmed strings inside an array of size 2.
 *
 * @param line      a null-terminated string
 * @param s         the length of s
 * @return on success an heap allocated array of size 2 containing pointers to 
 *         the parts before and after the first '=' char (excluded), which are 
 *         null-terminated, heap allocated, whitespace-trimmed strings. NULL is
 *         returned if @p line does not contain any '=' char.
 */
static char** const split_name_value(char const line[static 1], size_t s);


/**
 * Edit in place s removing any comments at the end of the line. This means
 * removing anything from the first occurrence of '#' to the end of the string
 * and whitespace trimming the remaining string on the right side. If the 
 * string does not contain any comment it is left unchanged.
 *
 * @param s         a null-terminated string
 */
static void remove_comment(char s[static 1]);


static char* expand_env(char const s[static 1]) {
    static char const pattern[] = "\\$\{.*?\\}";
    size_t slen = strlen(s);
    dynstr* expanded = dynstr_new();
    if (!expanded)
        exit(EXIT_FAILURE);
    
    pcre *regex;
    const char *error;
    int erroffset;
    size_t const MAX_OFFSETS = 30; // should be a multiple of 3. Take a look at
                                   // man 3 pcreapi
    int offsets[MAX_OFFSETS];

    regex = pcre_compile(pattern, 0, &error, &erroffset, NULL);
    if (!regex) {
        fprintf(stderr, "%s:%d: PCRE compilation failed at offset %d: %s\n", 
                __FILE__, __LINE__, erroffset, error);
        exit(EXIT_FAILURE);
    }

    int rc;
    int offset = 0;

    while ((rc = pcre_exec(regex, NULL, s, slen, offset, 0, offsets, MAX_OFFSETS)) >= 0) {
        int start = offsets[0];
        int end = offsets[1];

        // copy the previous part of the string
        if (dynstr_appendn(expanded, s + offset, start - offset))
            exit(EXIT_FAILURE);

        // expand variable
        size_t namelen = end - start - 2; // == (end - start + 1) - 3 to remove '${}'
        char name[namelen];
        snprintf(name, namelen, "%s", s + start + 2);
        char *var = getenv(name); 
        if (!var) {
            fprintf(stderr, "%s:%d: config variable \"%s\" does not exist\n", 
                    __FILE__, __LINE__, var);
            return NULL;
        }

        // add expanded variable
        if (dynstr_append(expanded, var))
            exit(EXIT_FAILURE);
        
        offset = offsets[1];
    }

    if (rc != PCRE_ERROR_NOMATCH && rc < 0) {
        fprintf(stderr, "%s:%d: PCRE matching failed: %d\n", __FILE__, __LINE__, rc);
        exit(EXIT_FAILURE);
    }

    if (dynstr_appendn(expanded, s + offset, slen - offset))
        exit(EXIT_FAILURE);

    return dynstr_tostr(expanded);
}


static Config* Config_init() {
    Config* pw_conf = calloc(1, sizeof(Config));
    if (!pw_conf) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__); 
        return NULL;
    }

    pw_conf->storage = NULL;
    pw_conf->gpg_fpr = NULL;

    return pw_conf;
}


void Config_free(Config config[static 1]) {
    if (config->storage) free((void*) config->storage);
    if (config->gpg_fpr) free((void*) config->gpg_fpr);
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfree-nonheap-object"
    free(config);
    #pragma GCC diagnostic pop
}


static VariableMapping** VariableMapping_init(Config* config) {
    VariableMapping** pw_mapping = calloc(CONFIG_LEN + 1, sizeof(VariableMapping*)); 
    if (!pw_mapping) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__); 
        return NULL;
    }

    VariableMapping_init_var(&pw_mapping[0], STRING, "storage", (void**) &config->storage, true);
    VariableMapping_init_var(&pw_mapping[1], STRING, "gpg_key", (void**) &config->gpg_fpr, true);

    pw_mapping[2] = NULL;   // null-terminating the array
    return pw_mapping;
}


static void VariableMapping_init_var(VariableMapping* v[static 1], enum Types type,
        char const name[static 1], void* config_ref[static 1], bool is_required) {
    size_t const sname = strlen(name);
    *v = calloc(1, sizeof(VariableMapping));
    if (!*v) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__); 
        exit(EXIT_FAILURE);
    }
    (*v)->name = calloc(sname + 1, sizeof(char));
    if (!(*v)->name) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__); 
        exit(EXIT_FAILURE);
    }
    (*v)->name = strncpy((*v)->name, name, sname);
    (*v)->var = config_ref;
    (*v)->type = type;
    (*v)->is_required = is_required;
}


static int VariableMapping_set(VariableMapping* mapping[static 1],
        char const name[static 1], void* value) {
    VariableMapping* elem;
    for (int i = 0; (elem = mapping[i]); i++) {
        if (strcmp(name, elem->name) == 0) {
            switch (elem->type) {
                case STRING:
                    VariableMapping_set_string(elem, value);
                    // ignoring unexistent environment variables (look at 
                    // VariableMapping_set_string() specification)
                    return 0;
                default: {
                    fprintf(stderr, "%s:%d: trying to assign to non-existent "
                            "Types value\n", __FILE__, __LINE__);
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    fprintf(stderr, "%s:%d: variable of name %s does not exist\n", __FILE__, 
            __LINE__, name);
    return -1;
}


static int VariableMapping_set_string(VariableMapping v[static 1], 
        char value[static 1]) {
    char* expanded = expand_env(value);
    if (!expanded) return -1;
    if (*v->var) 
        free(*v->var);
    *v->var = expanded; 
    return 0;
}


static void VariableMapping_defaults(VariableMapping** mapping) {
    if (VariableMapping_set(mapping, "storage", "${HOME}/.pwstore"))
        exit(EXIT_FAILURE);
    // gpg_fpr = NULL;
}


static void VariableMapping_free(VariableMapping* mapping[static 1]) {
    for (int i = 0; mapping[i]; i++) {
        free(mapping[i]->name);
        free(mapping[i]);
    }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfree-nonheap-object"
    free(mapping); 
    #pragma GCC diagnostic pop
}


static int parse_config_file(char const file_name[static 1],
        VariableMapping* mapping[static 1]) {
    FILE *f = fopen(file_name, "r");
    if (!f) {
        fprintf(stderr, "%s:%d: error while opening \"%s\" (errno: %d)\n", 
                __FILE__, __LINE__, file_name, errno); 
        return -1;
    }

    char* line = NULL;
    size_t line_size;
    size_t count;

    while ((count = getline(&line, &line_size, f) > 0)) {
        triml(line);
        if (line[0] == '#')
            continue;
        char** var = split_name_value(line, line_size);
        if (!var) 
            continue;
        remove_comment(var[1]);
        VariableMapping_set(mapping, var[0], var[1]);
        free(var[0]);
        free(var[1]);
        free(var);
    }

    free(line);
    fclose(f);
    return 0;
}


static char** const split_name_value(char const line[static 1], size_t s) {
    char* const equals = strchr(line, '=');
    if (!equals) 
        return NULL;

    char** const r = calloc(2, sizeof(char *));
    if (!r) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__); 
        exit(EXIT_FAILURE);
    }

    size_t namelen  = equals - line;
    size_t valuelen = s - namelen + 1;
    char* name  = calloc(namelen + 1, sizeof(char));
    char* value = calloc(valuelen + 1, sizeof(char));
    if (!name | !value) {
        fprintf(stderr, "%s:%d: allocation error\n", __FILE__, __LINE__); 
        exit(EXIT_FAILURE);
    }
    name  = strncpy(name, line, namelen);
    value = strncpy(value, line + namelen + 1, valuelen);
    trim(name);
    trim(value);
    r[0] = name;
    r[1] = value;
    return r;
}


static void remove_comment(char s[static 1]) {
    char* hash = strchr(s, (unsigned char) '#');
    if (!hash) 
        return;
    *hash = 0;
    trimr(s);
}


Config* read_config(char const* const path_list[static 1]) {
    Config* config = Config_init();
    VariableMapping** mapping = VariableMapping_init(config);
    if (!mapping)
        return NULL;

    VariableMapping_defaults(mapping);

    for (int i = 0; path_list[i]; i++)
        parse_config_file(path_list[i], mapping);

    VariableMapping_free(mapping);
    return config;
}
