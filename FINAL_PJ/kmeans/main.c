#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include "kmeans.h"

int main(int argc, char **argv) {
    srand(time(NULL));
    int i, j;
    int n_clusters=3;
    int max_iter=500;
    int run_times=1;
    double tol = 1e-4;
    double curr_score=DBL_MAX, best_score=DBL_MAX;
    kmeans_data data_pack;
    kmeans_centroids centroids, best_centroids;
    kmeans_labels labels, best_labels;
    FILE *fp=NULL;

    if (argc!=6) {
        fprintf(stderr, "Usage: kmeans data_path n_clusters tol max_iter run_times\n");
        exit(1);
    } 

    run_times = atol(argv[5]);
    max_iter = atol(argv[4]);
    tol = atof(argv[3]);
    n_clusters = atol(argv[2]);
    fprintf(stderr, "Setteings:\ntol: %.5f\nn_clusters: %d\npath: %s\nmax_iter: %d\nrun_times: %d\n", tol, n_clusters, argv[1], max_iter, run_times);
    fprintf(stderr, "Loading data...\n");
    load_kmeans_data(argv[1], &data_pack);
    fprintf(stderr, "Data shape: (%d,%d)\n", data_pack.n_rows, data_pack.n_cols);

    fprintf(stderr, "Training...\n");

    for (int i=0; i<run_times; ++i) {

        fprintf(stderr, "=== %d/%d Run ===\n", i+1, run_times);

        centroids.n_rows = n_clusters;
        centroids.n_cols = data_pack.n_cols;
        centroids.data   = NULL;
        centroids._data  = NULL;
        labels.n_labels  = data_pack.n_rows;
        labels.data      = NULL;

        curr_score = kmeans_intersec_int(data_pack.data, &labels.data, &centroids.data, data_pack.n_rows, data_pack.n_cols, n_clusters, tol, max_iter, 1);
        if (i==0 || curr_score<best_score) { // if first time or better solution, update best
            if (i!=0) { // if old best exists, delete old best
                destroy_kmeans_centroids(&best_centroids);
                destroy_kmeans_labels(&best_labels);
            }
            best_score = curr_score;
            memcpy(&best_centroids, &centroids, sizeof(centroids));
            memcpy(&best_labels, &labels, sizeof(labels));
        } else { // not a better solution, delete it
            destroy_kmeans_centroids(&centroids);
            destroy_kmeans_labels(&labels);
        }
    }

    fprintf(stderr, "Training finished.\n");
    fprintf(stderr, "best_score(lower is better): %.2f\n", best_score);
    destroy_kmeans_data(&data_pack);

    fp = fopen("./labels.csv", "w");
    fprintf(fp, "id,labels\n");
    for (unsigned int i=0; i<best_labels.n_labels; ++i) {
        fprintf(fp, "%u,%u\n", i, best_labels.data[i]);
    }
    fclose(fp); fp=NULL;

    write_kmeans_labels("./labels.bin", &best_labels);
    destroy_kmeans_labels(&best_labels);
    write_kmeans_centroids("./centroids.bin", &best_centroids);
    destroy_kmeans_centroids(&best_centroids);
    return 0;
}
