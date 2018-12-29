#ifndef __INCLUDE_SAIS_HEADER__
#define __INCLUDE_SAIS_HEADER__

#ifndef UCHAR_SIZE
# define UCHAR_SIZE 256
#endif
#ifndef MINBUCKETSIZE
# define MINBUCKETSIZE 256
#endif

#define sais_index_type int
#define sais_bool_type  int
#define SAIS_LMSSORT2_LIMIT 0x3fffffff

#define SAIS_MYMALLOC(_num, _type) ((_type *)malloc((_num) * sizeof(_type)))
#define chr(_a) (cs == sizeof(sais_index_type) ? ((sais_index_type *)T)[(_a)] : ((unsigned char *)T)[(_a)])

/* find the start or end of each bucket */
void getCounts(const void *T,sais_index_type *C,sais_index_type n,sais_index_type k,int cs);
void getBuckets(const sais_index_type *C,sais_index_type *B,sais_index_type k,sais_bool_type end);

/* sort all type LMS suffixes */
void LMSsort1(const void *T,sais_index_type *SA,sais_index_type *C,sais_index_type *B,sais_index_type n,sais_index_type k,int cs);
sais_index_type LMSpostproc1(const void *T,sais_index_type *SA,sais_index_type n,sais_index_type m,int cs);
void LMSsort2(const void *T,sais_index_type *SA,sais_index_type *C,sais_index_type *B,sais_index_type *D,sais_index_type n,sais_index_type k,int cs);
sais_index_type LMSpostproc2(sais_index_type *SA,sais_index_type n,sais_index_type m);

/* compute SA and BWT */
void induceSA(const void *T,sais_index_type *SA,sais_index_type *C,sais_index_type *B,sais_index_type n,sais_index_type k,int cs);
sais_index_type computeBWT(const void *T,sais_index_type *SA,sais_index_type *C,sais_index_type *B,sais_index_type n,sais_index_type k,int cs);

/* find the suffix array SA of T[0..n-1] in {0..255}^n */
sais_index_type sais_main(const void *T,sais_index_type *SA,sais_index_type fs,sais_index_type n,sais_index_type k,int cs,sais_bool_type isbwt);

/*---------------------------------------------------------------------------*/

int sais_int(const int *T,int *SA,int n,int k);
int sais_bwt(const unsigned char *T,unsigned char *U,int *A,int n);
int sais_int_bwt(const int *T,int *U,int *A,int n,int k);
int sais(const unsigned char *T,int *SA,int n);
#endif

