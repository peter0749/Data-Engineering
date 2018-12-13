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
void *kmeans_intersec_int(unsigned int **data, unsigned int **return_labels, double ***return_centroid, int rows, int cols, int K, double tol);
inline double hist_intersection(unsigned int *P, double *Q, unsigned int cols) {
    double P_M = 0.0;
    double Q_M = 0.0;
    double JSD = 0.0;
    for (unsigned int i=0; i<cols; ++i) {
        double M_i = ((double)P[i]+Q[i]) / 2.0;
        P_M += (double)P[i]*log((double)P[i] / M_i);
        Q_M +=         Q[i]*log(        Q[i] / M_i);
    }
    JSD = (P_M+Q_M) / 2.0;
    return JSD*JSD;
}

inline double hist_intersection_f(double *P, double *Q, unsigned int cols) {
    double P_M = 0.0;
    double Q_M = 0.0;
    double JSD = 0.0;
    for (unsigned int i=0; i<cols; ++i) {
        double M_i = (P[i]+Q[i]) / 2.0;
        P_M += P[i]*log(P[i] / M_i);
        Q_M += Q[i]*log(Q[i] / M_i);
    }
    JSD = (P_M+Q_M) / 2.0;
    return JSD*JSD;
}

#endif
