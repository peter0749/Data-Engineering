#include <pthread.h>
#include <omp.h>
#include "kmeans.h"

double hist_intersection(unsigned int *A, double *B, unsigned int cols) {
    unsigned int intersect=0;
    unsigned int onions=0;
    for (unsigned int i=0; i<cols; ++i) {
        intersect += (A[i]<B[i]?A[i]:B[i]);
        onions += (A[i]>B[i]?A[i]:B[i]);
    }
    return 1.0 - (double)(intersect+1) / (double)(onions+1);
}

double hist_intersection_f(double *A, double *B, unsigned int cols) {
    unsigned int intersect=0;
    unsigned int onions=0;
    for (unsigned int i=0; i<cols; ++i) {
        intersect += (A[i]<B[i]?A[i]:B[i]);
        onions += (A[i]>B[i]?A[i]:B[i]);
    }
    return 1.0 - (double)(intersect+1) / (double)(onions+1);
}

void *kmeans_intersec_int(unsigned int **data, unsigned int **return_labels, double ***return_centroid, int rows, int cols, int K, double tol) {
    double mean_centroid_d = DBL_MAX;
    double **centroids = NULL;
    double **new_centroids = NULL;
    unsigned int *lab_counts=NULL;
    int *labels=NULL;
    centroids = (double**)malloc(sizeof(double*)*K);
    new_centroids = (double**)malloc(sizeof(double*)*K);
    labels = (int*)malloc(sizeof(int)*rows);
    lab_counts = (int*)malloc(sizeof(int)*K);

    for (int i=0; i<K; ++i) centroids[i] = (double*)malloc(sizeof(double)*cols);
    for (int i=0; i<K; ++i) new_centroids[i] = (double*)malloc(sizeof(double)*cols);

    // initialize (randomly pick k samples wo replacement) 
    unsigned int _h=rand(); 
    for (int i=0; i<K; ++i) {
        _h = _h%rows;
        for (int j=0; j<cols; ++j) {
            centroids[i][j] = (double)data[_h][j];
        }
        _h += 101111; // prime
    }

    while(mean_centroid_d>tol) {
        // determine labels
        #pragma omp parallel for
        for (int i=0; i<rows; ++i) {
            unsigned int best_l=0;
            double min_d=DBL_MAX;
            for (int k=0; k<K; ++k) {
                double d = hist_intersection(data[i], centroids[k], cols);
                if (d<min_d) {
                    min_d = d;
                    best_l = k;
                }
            }
            labels[i] = best_l;
        }
        // determine new centroids
        for (int k=0; k<K; ++k) memset(new_centroids[k], 0x00, sizeof(double)*cols);
        memset(lab_counts, 0x00, sizeof(int)*K);
        for (int i=0; i<rows; ++i) {
            unsigned int l = labels[i];
            ++lab_counts[l];
            for (int j=0; j<cols; ++j) {
                new_centroids[l][j] += data[i][j]; // sum
            }
        }
        for (int k=0; k<K; ++k) {
            for (int j=0; j<cols; ++j) {
                new_centroids[k][j] /= (double)(lab_counts[k]+1e-8); // mean
            }
        }
        mean_centroid_d = 0;
        for (int k=0; k<K; ++k) {
            mean_centroid_d += hist_intersection_f(centroids[k], new_centroids[k], cols);
        }
        mean_centroid_d /= (double)K;
        // assign new centroids to centroids
        for (int k=0; k<K; ++k) {
            double *ptr = centroids[k];
            centroids[k] = new_centroids[k];
            new_centroids[k] = ptr;
        }
#ifdef __DEBUG_K_MEANS
        fprintf(stderr, "%.2lf\n", mean_centroid_d);
#endif
    }

    for (int k=0; k<K; ++k) free(new_centroids[k]);

    free(lab_counts);
    free(new_centroids);

    if(return_centroid!=NULL) *return_centroid = centroids;
    else free(centroids);
    if(return_labels!=NULL)   *return_labels   = labels;
    else free(labels);
}
