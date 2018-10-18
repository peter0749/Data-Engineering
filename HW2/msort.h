/*
 * A very simple implementation of parallel merge sort
 */
#ifndef __INCLUDE_MSORT_HEADER
#define __INCLUDE_MSORT_HEADER
#include <pthread.h>

typedef struct{
    char const *array;
    char const *buffer;
    size_t block;
    int L, R;
    int (*cmp)(const void *, const void *);
    int depth;
    int MAXDEPTH;
}MSORT_ARGS;

void mymergesort(void *, const size_t , const size_t , const int, int (*cmp)(const void *, const void *));

#endif
