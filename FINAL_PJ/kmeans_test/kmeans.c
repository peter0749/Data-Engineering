#include <pthread.h>
#include <omp.h>
#include "kmeans.h"

void destroy_kmeans_labels(kmeans_labels *data_pak) {
    if (data_pak==NULL || data_pak->data==NULL) return;
    free(data_pak->data); data_pak->data = NULL;
    data_pak->n_labels=0;
}

void load_kmeans_labels(const char *fpath, kmeans_labels *data_pak) {
    FILE *fp=NULL;
    fp = fopen(fpath, "rb");
    unsigned int *content=NULL;
    unsigned int n_labels=0;
    fread(&n_labels, sizeof(unsigned int), 1, fp);
    content = (unsigned int*)malloc(sizeof(unsigned int)*n_labels);
    fread(content, sizeof(unsigned int), n_labels, fp);
    data_pak->n_labels = n_labels;
    data_pak->data = content;
    fclose(fp);
}

void write_kmeans_labels(const char *fpath, const kmeans_labels *data_pak) {
    FILE *fp=NULL;
    fp = fopen(fpath, "wb");
    fwrite(&data_pak->n_labels, sizeof(unsigned int), 1, fp);
    fwrite(data_pak->data, sizeof(unsigned int), data_pak->n_labels, fp);
    fclose(fp);
}

void destroy_kmeans_centroids(kmeans_centroids *data_pak) {
    if (data_pak==NULL) return;
    if (data_pak->data!=NULL) free(data_pak->data); data_pak->data = NULL;
    if (data_pak->_data!=NULL) free(data_pak->_data); data_pak->_data = NULL;
    data_pak->n_rows = data_pak->n_cols = 0;
}

void load_kmeans_centroids(const char *fpath, kmeans_centroids *data_pak) {
    FILE *fp=NULL;
    fp = fopen(fpath, "rb");
    double *content=NULL;
    unsigned int n_rows=0, n_cols=0;
    fread(&n_rows, sizeof(unsigned int), 1, fp);
    fread(&n_cols, sizeof(unsigned int), 1, fp);
    content = (double*)malloc(sizeof(double)*n_rows*n_cols);
    fread(content, sizeof(double), n_rows*n_cols, fp);
    data_pak->n_rows = n_rows;
    data_pak->n_cols = n_cols;
    data_pak->_data = content;
    data_pak->data = NULL;
    data_pak->data = (double**)malloc(sizeof(double*)*n_rows);
    for (unsigned int i=0; i<n_rows; ++i) data_pak->data[i] = content+i*n_cols;
    fclose(fp);
}

void write_kmeans_centroids(const char *fpath, const kmeans_centroids *data_pak) {
    FILE *fp=NULL;
    fp = fopen(fpath, "wb");
    fwrite(&data_pak->n_rows, sizeof(unsigned int), 1, fp);
    fwrite(&data_pak->n_cols, sizeof(unsigned int), 1, fp);
    // fwrite(data_pak->_data, sizeof(double), data_pak->n_rows*data_pak->n_cols, fp);
    for (unsigned int i=0; i<data_pak->n_rows; ++i) {
        fwrite(data_pak->data[i], sizeof(double), data_pak->n_cols, fp);
    }
    fclose(fp);
}

void destroy_kmeans_data(kmeans_data *data_pak) {
    if (data_pak==NULL) return;
    if (data_pak->data!=NULL) free(data_pak->data); data_pak->data = NULL;
    if (data_pak->_data!=NULL) free(data_pak->_data); data_pak->_data = NULL;
    data_pak->n_rows = data_pak->n_cols = 0;
}

void load_kmeans_data(const char *fpath, kmeans_data *data_pak) {
    FILE *fp=NULL;
    fp = fopen(fpath, "rb");
    unsigned int *content=NULL;
    unsigned int n_rows=0, n_cols=0;
    fread(&n_rows, sizeof(unsigned int), 1, fp);
    fread(&n_cols, sizeof(unsigned int), 1, fp);
    content = (unsigned int*)malloc(sizeof(unsigned int)*n_rows*n_cols);
    fread(content, sizeof(unsigned int), n_rows*n_cols, fp);
    data_pak->n_rows = n_rows;
    data_pak->n_cols = n_cols;
    data_pak->_data = content;
    data_pak->data = NULL;
    data_pak->data = (unsigned int**)malloc(sizeof(unsigned int*)*n_rows);
    for (unsigned int i=0; i<n_rows; ++i) data_pak->data[i] = content+i*n_cols;
    fclose(fp);
}

/*
double hist_intersection(unsigned int *P, double *Q, unsigned int cols) {
    double P_M = 0.0;
    double Q_M = 0.0;
    double JSD = 0.0;
    for (unsigned int i=0; i<cols; ++i) {
        double M_i = ((double)P[i]+Q[i]) / 2.0 + 1e-8;
        P_M += (double)P[i]*log(((double)P[i]+1e-8) / M_i);
        Q_M +=         Q[i]*log((        Q[i]+1e-8) / M_i);
    }
    JSD = (P_M+Q_M) / 2.0;
    return JSD*JSD;
}

double hist_intersection_f(double *P, double *Q, unsigned int cols) {
    double P_M = 0.0;
    double Q_M = 0.0;
    double JSD = 0.0;
    for (unsigned int i=0; i<cols; ++i) {
        double M_i = (P[i]+Q[i]) / 2.0 + 1e-8;
        P_M += P[i]*log((P[i]+1e-8) / M_i);
        Q_M += Q[i]*log((Q[i]+1e-8) / M_i);
    }
    JSD = (P_M+Q_M) / 2.0;
    return JSD*JSD;
}
*/

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

void *kmeans_intersec_int(unsigned int **data, unsigned int **return_labels, double ***return_centroid, int rows, int cols, int K, double tol, int max_iter, double noise_amp, double delta, char verbose) {
    double mean_centroid_d = DBL_MAX;
    double **centroids = NULL;
    double **new_centroids = NULL;
    unsigned int *lab_counts=NULL;
    unsigned int iter_counter=0;
    int *labels=NULL;
    centroids = (double**)malloc(sizeof(double*)*K);
    new_centroids = (double**)malloc(sizeof(double*)*K);
    labels = (int*)malloc(sizeof(int)*rows);
    lab_counts = (int*)malloc(sizeof(int)*K);

    for (int i=0; i<K; ++i) centroids[i] = (double*)malloc(sizeof(double)*cols);
    for (int i=0; i<K; ++i) new_centroids[i] = (double*)malloc(sizeof(double)*cols);

    // initialize (randomly pick k samples wo replacement) 
    for (int i=0; i<K; ++i) {
        int l=rand()%rows;
        // check repetition
        char fail;
        do {
            fail=0;
            for (int j=0; j<i; ++j) 
                if (lab_counts[j]==l) {
                    fail=1;
                    l=rand()%rows; // pick another
                    break;
                }
        } while(fail);
        lab_counts[i] = l;
        for (int j=0; j<cols; ++j) {
            centroids[i][j] = (double)data[l][j];
        }
    }

    iter_counter=0;
    while(iter_counter<max_iter && mean_centroid_d>tol) {
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
                new_centroids[k][j] += ((double)rand() / (double)RAND_MAX * 2.0 - 1.0) * noise_amp; // noise = [-1,+1] * noise_amp (0-mean)
                if (new_centroids[k][j]<0) new_centroids[k][j]=0;
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
        ++iter_counter;
        if (verbose) fprintf(stderr, "[%d] d:%.4lf, n:%.4lf\n", iter_counter, mean_centroid_d, noise_amp);
        noise_amp *= delta;
    }

    for (int k=0; k<K; ++k) free(new_centroids[k]);

    free(lab_counts);
    free(new_centroids);

    if(return_centroid!=NULL) *return_centroid = centroids;
    else free(centroids);
    if(return_labels!=NULL)   *return_labels   = labels;
    else free(labels);
}
