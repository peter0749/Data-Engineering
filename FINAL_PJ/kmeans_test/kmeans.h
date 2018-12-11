#ifndef __INCLUDE_KMEANS_H
#define __INCLUDE_KMEANS_H
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#ifndef DBL_MAX
#define DBL_MAX 2.0e9
#endif
#ifndef DBL_MIN
#define DBL_MIN -2.0e9
#endif
double hist_intersection(unsigned int *A, double *B, unsigned int cols);
void *kmeans_intersec_int(unsigned int **data, unsigned int **return_labels, double ***return_centroid, int rows, int cols, int K, double tol);
double hist_intersection_f(double *A, double *B, unsigned int cols);
#endif
