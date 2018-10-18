#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    srand(time(NULL));
    unsigned long N=atol(argv[1]);
    for (unsigned long i=0; i<N; ++i) {
        fprintf(stdout, "%010u\n", (unsigned)rand());
    }
    return 0;
}
