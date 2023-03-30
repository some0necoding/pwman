#ifndef PATH_UTIL
#define PATH_UTIL

#include <sys/types.h>

/* Builds {root}/{rel_path} */
const char *build_path(const char *root, const char *rel_path);

/* Adds extension ext to fname */
const char *add_ext(const char *fname, const char *ext);

/* Removes extension ext from fname */
const char *rm_ext(const char *fname, const char* ext);

/* Recursively call mkdirat on all directories of path */
int psm_mkdir(const char *relative_path, mode_t mode);

#endif
