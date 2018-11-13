#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char **argv) {
    srand(time(NULL));
    unsigned long N=atol(argv[1]);
    for (unsigned long i=0; i<N; ++i) {
        for (unsigned char n=0; n<10; ++n)
            fprintf(stdout, "%c", rand()%26+'a');
        fprintf(stdout, "\n");
    }
    return 0;
}
