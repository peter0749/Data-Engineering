#ifndef __INCLUDE_KMEANS_H
#define __INCLUDE_KMEANS_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <string.h>
#include <float.h>
//#ifndef DBL_MAX
//#define DBL_MAX 2.0e9
//#endif
//#ifndef DBL_MIN
//#define DBL_MIN -2.0e9
//#endif

typedef struct {
    unsigned int n_rows;
    unsigned int n_cols;
    unsigned int *_data;
    unsigned int **data;
} kmeans_data;

typedef struct {
    unsigned int n_rows;
    unsigned int n_cols;
    double *_data;
    double **data;
} kmeans_centroids;

typedef struct {
    unsigned int n_labels;
    unsigned int *data;
} kmeans_labels;

void destroy_kmeans_labels(kmeans_labels *data_pak);
void load_kmeans_labels(const char *fpath, kmeans_labels *data_pak);
void write_kmeans_labels(const char *fpath, const kmeans_labels *data_pak);
void destroy_kmeans_centroids(kmeans_centroids *data_pak);
void load_kmeans_centroids(const char *fpath, kmeans_centroids *data_pak);
void write_kmeans_centroids(const char *fpath, const kmeans_centroids *data_pak);
void destroy_kmeans_data(kmeans_data *data_pak);
void load_kmeans_data(const char *fpath, kmeans_data *data_pak);
double hist_intersection(unsigned int *P, double *Q, unsigned int cols);
double hist_intersection_f(double *P, double *Q, unsigned int cols);

void *kmeans_intersec_int(unsigned int **data, unsigned int **return_labels, double ***return_centroid, int rows, int cols, int K, double tol, int max_iter, double noise_amp, double delta, char verbose);

#endif
