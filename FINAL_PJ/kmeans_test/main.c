#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "kmeans.h"

int main(void) {
    srand(time(NULL));
    int i, j;
    unsigned int **data = NULL;
    double **centroids = NULL;
    data = (unsigned int**)malloc(sizeof(int*)*30000);
    for (i=0; i<30000; ++i) data[i] = (unsigned int*)malloc(sizeof(int)*3);
    for (i=0; i<10000; ++i) {
        data[i][0] = rand()%49;     // [  0,  48], mean=24
        data[i][1] = rand()%51+100; // [100, 150], mean=125
        data[i][2] = rand()%51+20;  // [ 20,  70], mean=45
    }
    for (i=10000; i<20000; ++i) {
        data[i][0] = rand()%51+70;  // [70,  120], mean=95
        data[i][1] = rand()%49;     // [ 0,   48], mean=24
        data[i][2] = rand()%51+100; // [100, 150], mean=125
    }
    for (i=20000; i<30000; ++i) {
        data[i][0] = rand()%51+30; // [30,  80], mean=55
        data[i][1] = rand()%51+50; // [50, 100], mean=75
        data[i][2] = rand()%49;   //  [ 0,  48], mean=24
    }
    kmeans_intersec_int(data, NULL, &centroids, 30000, 3, 3, 1e-5);
    /*
     * expect 3 clusters:
     * (24,125,45), (95,24,125), (55,75,24)
     * approximately
     */
    for (i=0; i<3; ++i) {
        fprintf(stderr, "%d: %.2lf %.2lf %.2lf\n", i, centroids[i][0], centroids[i][1], centroids[i][2]);
    }
    for (i=0; i<3; ++i) {
        free(centroids[i]);
    }
    free(centroids);
    for (i=0; i<30000; ++i) free(data[i]);
    free(data);
    return 0;
}
