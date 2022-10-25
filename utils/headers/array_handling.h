#ifndef ARRAY_HANDLING
#define ARRAY_HANDLING

// this function returns the length of a null-terminated 2D array (for 1D arrays already exists strlen())
int arrlen(void **arr);

// checking for correct allocation (it works only if malloc, calloc, realloc are used)
int check_allocation(void *arr) 

#endif