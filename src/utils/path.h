#ifndef PATH_UTIL
#define PATH_UTIL

/* Builds {root}/{rel_path} */
const char *build_path(const char *root, const char *rel_path);

/* Adds extention ext to fname */
const char *add_ext(const char *fname, const char *ext);

#endif
