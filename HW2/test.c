#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    srand(time(NULL));
    unsigned long N=100000000;
    for (unsigned long i=0; i<N; ++i) {
        fprintf(stdout, "%lu\n", (unsigned)rand());
    }
    return 0;
}
