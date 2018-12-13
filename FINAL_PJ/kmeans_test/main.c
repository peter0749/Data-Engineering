#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "kmeans.h"

int main(void) {
    srand(time(NULL));
    int i, j;
    unsigned int **data = NULL;
    double **centroids = NULL;
    data = (unsigned int**)malloc(sizeof(int*)*30000000);
    for (i=0; i<30000000; ++i) data[i] = (unsigned int*)malloc(sizeof(int)*4);
    for (i=0; i<10000000; ++i) {
        data[i][0] = rand()%49;     // [  0,  48], mean=24
        data[i][1] = rand()%51+100; // [100, 150], mean=125
        data[i][2] = rand()%51+20;  // [ 20,  70], mean=45
        data[i][3] = rand()%51+70;  // [70,  120], mean=95
    }
    for (i=10000000; i<20000000; ++i) {
        data[i][0] = rand()%51+70;  // [70,  120], mean=95
        data[i][1] = rand()%49;     // [ 0,   48], mean=24
        data[i][2] = rand()%51+100; // [100, 150], mean=125
        data[i][3] = rand()%51+30; // [30,  80], mean=55
    }
    for (i=20000000; i<30000000; ++i) {
        data[i][0] = rand()%51+30; // [30,  80], mean=55
        data[i][1] = rand()%51+50; // [50, 100], mean=75
        data[i][2] = rand()%49;   //  [ 0,  48], mean=24
        data[i][3] = rand()%51+100; // [100, 150], mean=125
    }
    /*
    for (i=0; i<30000000; ++i) { // add variance
        int d = rand()%15 - 7; // [-7,+7]
        data[i][0] += (unsigned int)(d<0?0:d);  
        d = rand()%21 - 10; // [-10,+10]
        data[i][1] += (unsigned int)(d<0?0:d); 
        d = rand()%9 - 4; // [-4,+4]
        data[i][2] += (unsigned int)(d<0?0:d); 
        d = rand()%31 - 15; // [-15,+15]
        data[i][3] += (unsigned int)(d<0?0:d); 
    }
    */
    kmeans_intersec_int(data, NULL, &centroids, 30000000, 4, 3, 1e-4);
    /*
     * expect 3 clusters:
     * (24,125,45,95), (95,24,125,55), (55,75,24,125)
     * approximately
     */
    for (i=0; i<3; ++i) {
        fprintf(stderr, "%d: %.2lf %.2lf %.2lf %.2lf\n", i, centroids[i][0], centroids[i][1], centroids[i][2], centroids[i][3]);
    }
    for (i=0; i<3; ++i) {
        free(centroids[i]);
    }
    free(centroids);
    for (i=0; i<30000000; ++i) free(data[i]);
    free(data);
    return 0;
}
