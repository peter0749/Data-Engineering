#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "kmeans.h"

int main(int argc, char **argv) {
    srand(time(NULL));
    int i, j;
    int n_clusters=3;
    double tol = 1e-4;
    kmeans_data data_pack;
    kmeans_centroids centroids_struct;
    kmeans_labels labels_struct;
    FILE *fp=NULL;

    if (argc!=4) {
        fprintf(stderr, "Usage: kmeans data_path n_clusters tol\n");
        exit(1);
    } 

    tol = atof(argv[3]);
    n_clusters = atol(argv[2]);
    fprintf(stderr, "Setteings:\ntol: %.5f\nn_clusters: %d\npath: %s\n", tol, n_clusters, argv[1]);
    fprintf(stderr, "Loading data...\n");
    load_kmeans_data(argv[1], &data_pack);
    fprintf(stderr, "Data shape: (%d,%d)\n", data_pack.n_rows, data_pack.n_cols);

    centroids_struct.n_rows = n_clusters;
    centroids_struct.n_cols = data_pack.n_cols;
    centroids_struct.data   = NULL;
    centroids_struct._data  = NULL;
    labels_struct.n_labels  = data_pack.n_rows;
    labels_struct.data      = NULL;

    fprintf(stderr, "Training...\n");
    kmeans_intersec_int(data_pack.data, &labels_struct.data, &centroids_struct.data, data_pack.n_rows, data_pack.n_cols, n_clusters, tol, 20, 1);
    fprintf(stderr, "Training finished.\n");
    destroy_kmeans_data(&data_pack);

    fp = fopen("./labels.csv", "w");
    fprintf(fp, "labels\n");
    for (unsigned int i=0; i<labels_struct.n_labels; ++i) {
        fprintf(fp, "%u\n", labels_struct.data[i]);
    }
    fclose(fp); fp=NULL;

    write_kmeans_labels("./labels.bin", &labels_struct);
    destroy_kmeans_labels(&labels_struct);
    write_kmeans_centroids("./centroids.bin", &centroids_struct);
    destroy_kmeans_centroids(&centroids_struct);
    return 0;
}
