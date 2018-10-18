/*
 * A very simple implementation of parallel merge sort
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#ifdef __APPLE__
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include "msort.h"

// both array, and temp are shared memory
void *devide_thread(void *);
inline void merge(char const *array, char const *temp, const size_t block, const int L, const int M, const int R, int (*cmp)(const void *a, const void *b)) __attribute__((always_inline));

inline void merge(char const *array, char const *temp, const size_t block, const int L, const int M, const int R, int (*cmp)(const void *a, const void *b)) {
    int i, j, k, t;
    // char *temp = (char*)malloc(block*(R-L+1));
    i=L; j=M+1; k=L;
    while(i<=M && j<=R){
        t = cmp(array+block*i, array+block*j);
        if(t<0){
            memcpy((void*)(temp+k*block), (void*)(array+block*i), block);
            ++i;
        }else{
            memcpy((void*)(temp+k*block), (void*)(array+block*j), block);
            ++j;
        }
        ++k;
    }
    if (i<=M) memcpy((void*)(temp+k*block), (void*)(array+block*i), block*(M-i+1));
    if (j<=R) memcpy((void*)(temp+k*block), (void*)(array+block*j), block*(R-j+1));
    memcpy((void*)(array+block*L), (void*)(temp+block*L), block*(R-L+1)); // write back
    // free(temp);
    //pass
}
void devide(char const *array , char const *buffer, const size_t block, const int L, const int R, int (*cmp)(const void *a, const void *b) ,const int depth, const int MAXDEPTH){
    //pass
    if(L==R)return;
    int M=(L+R)>>1;
    if(depth >= MAXDEPTH) {
        qsort((void*)(array+L*block), R-L+1, block, cmp);
        return;
    }
    MSORT_ARGS *arg1 = NULL;
#ifndef __APPLE__
    arg1 = (MSORT_ARGS*)aligned_alloc(sizeof(MSORT_ARGS), sizeof(MSORT_ARGS));
#else
    arg1 = (MSORT_ARGS*)valloc(sizeof(MSORT_ARGS));
#endif
    assert(arg1!=NULL);
    MSORT_ARGS *arg2 = NULL;
#ifndef __APPLE__

    arg2 = (MSORT_ARGS*)aligned_alloc(sizeof(MSORT_ARGS), sizeof(MSORT_ARGS));
#else
    arg2 = (MSORT_ARGS*)valloc(sizeof(MSORT_ARGS));
#endif
    assert(arg2!=NULL);
    arg1->array = array;
    arg1->buffer = buffer;
    arg1->block = block;
    arg1->L = L;
    arg1->R = M;
    arg1->cmp = cmp;
    arg1->depth = depth+1;
    arg1->MAXDEPTH = MAXDEPTH;
    arg2->array = array;
    arg2->buffer = buffer;
    arg2->block = block;
    arg2->L = M+1;
    arg2->R = R;
    arg2->cmp = cmp;
    arg2->depth = depth+1;
    arg2->MAXDEPTH = MAXDEPTH;
    pthread_t *thread1=NULL, *thread2=NULL;
    thread1 = (pthread_t*)malloc(sizeof(pthread_t));
    assert(thread1!=NULL);
    thread2 = (pthread_t*)malloc(sizeof(pthread_t));
    assert(thread2!=NULL);
    int re1=0, re2=0;
    if((re1=(pthread_create(thread1, NULL, devide_thread, (void*)(arg1))))) {
        qsort((void*)(array+L*block), M-L+1, block, cmp);
    }
    if((re2=(pthread_create(thread2, NULL, devide_thread, (void*)(arg2))))) {
        qsort((void*)(array+(M+1)*block), R-M, block, cmp);
    }
    if (!re1)
        pthread_join(*thread1, NULL);
    if (!re2)
        pthread_join(*thread2, NULL);
    free(arg1), free(arg2);
    free(thread1), free(thread2);
    merge(array, buffer, block, L, M, R, cmp);
}

void *devide_thread(void *init){
    MSORT_ARGS *s = (MSORT_ARGS*)init;
    devide(s->array,s->buffer,s->block,s->L,s->R,s->cmp,s->depth, s->MAXDEPTH);
    pthread_exit(0);
    return NULL;
}

void mymergesort(void *array, const size_t num, const size_t block, const int MAXCORE, int (*cmp)(const void *a, const void *b)){
    int MAXDEPTH = ceil(log2(MAXCORE));
#ifndef __APPLE__
    char *buffer = (char*)aligned_alloc(block,num*block);
#else
    char *buffer = (char*)valloc(num*block);
#endif
    devide((char*)array, buffer, block, 0, num-1, cmp, 0, MAXDEPTH);
    free(buffer);
}
