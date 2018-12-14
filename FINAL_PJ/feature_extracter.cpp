#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <locale>
#include <cassert>
#include "read_histogram.hpp"

int main(int argc, char **argv) {
    using namespace std;
    wchar_t *read_buff=NULL;
    char fpath[128];
    wstring key_s, read_line, sub;
    unsigned int value=0;
    unsigned int skipN=100;
    unsigned int n_records=0;
    FILE *fp = NULL;
    unordered_map<wstring, unsigned int> dict;
    unordered_map<wstring, unsigned int> word2class;
    unsigned int n_class=0;
    if (argc!=2) exit(1);
    skipN = atol(argv[1]);
    setlocale(LC_ALL, "");

    read_buff = new wchar_t[512];

    read_words("./.db/word_count/total.cnt", skipN, read_buff, 511, dict);
    word2class = get_histogram_mapping("./.db/word2vec/classes.txt", &n_class, read_buff, 511);

    fp = fopen("./.db/counts", "r");
    fscanf(fp, "%u", &n_records);
    fclose(fp); fp=NULL;

    fp = fopen("./.db/word_count/features.bin", "wb");
    fwrite(&n_records, sizeof(unsigned int), 1, fp);
    fwrite(&n_class, sizeof(unsigned int), 1, fp);

    unsigned int percentage = 0, last_percentage = 0, done_workers = 0;
    for (unsigned int i=0; i<n_records; ++i) {
        unordered_map<wstring, unsigned int> local_map;
        unsigned int *feature = NULL;

        sprintf(fpath, "./.db/word_count/%d.cnt", i);
        local_map = read_histogram(fpath, read_buff, 511);

        feature = hist2vec(local_map, word2class, n_class);
        fwrite(feature, sizeof(unsigned int), n_class, fp);
        delete[] feature; feature=NULL;
        ++done_workers;
        percentage = (unsigned int)((float)(done_workers)/(float)n_records*100);
        if (percentage>last_percentage) {
            fprintf(stderr, "\rprogress: %3u%%", percentage);
            fflush(stderr);
            last_percentage = percentage;
        }
    }
    delete[] read_buff; read_buff=NULL;
    assert(done_workers==n_records);

    fclose(fp); fp=NULL;

    fp = fopen("./.db/word_count/skip_cols.txt", "w");
    fprintf(fp, "%u\n", skipN);
    fclose(fp); fp=NULL;

    fprintf(stderr, "\nDone!\n");
    fflush(stderr);

    return 0;
}
