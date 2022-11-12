#ifndef PATH_UTIL
#define PATH_UTIL

/* Builds {root}/{rel_path} */
int build_path(char **root, char *rel_path);

/* Adds extention ext to fname */
char *add_ext(char *fname, const char *ext);

#endif