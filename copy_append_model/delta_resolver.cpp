#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "filereader.hpp"

int main(int argc, char **argv) {
    unsigned char *fileA=NULL;
    char *buffer=NULL;
    unsigned char op;
    int l, r;
    int buffer_size = 100;
    int fileA_len=0;
    FILE *fpDelta=NULL;
    if (argc!=4) {
        fprintf(stderr ,"Usage: ./delta_resolver fileA delta buffer_size(in MB)\n");
        exit(1);
    }
    buffer_size = atol(argv[3]);
    buffer_size *= (long long)1024*1024;
    buffer = new char[buffer_size];
    fileA_len = open_n_read_file(argv[1], &fileA);
    fpDelta = fopen(argv[2], "rb");

    while((fgets(buffer, buffer_size-1, fpDelta))!=NULL) {
        l=r=-1;
        sscanf(buffer, "%1c %d,%d", &op, &l, &r);
        switch(op) {
            case 'a':
                fread(buffer, sizeof(char), l, fpDelta);
                fwrite(buffer, sizeof(char), l, stdout);
                break;
            case 'c':
                fwrite(fileA+l, sizeof(unsigned char), r, stdout);
                break;
            default:
                fprintf(stderr, "An ERROR occured! Are the files inconsistant?\n");
                break;
        }
    }
    
    fclose(fpDelta);
    delete[] buffer;
}
