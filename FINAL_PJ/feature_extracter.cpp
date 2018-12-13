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
    wchar_t read_buff[256];
    char fpath[128];
    wstring key_s, read_line, sub;
    unsigned int value=0;
    unsigned int topN=5000, n_features=0;
    unsigned int skipN=100;
    unsigned int n_records=0;
    FILE *fp = NULL;
    unordered_map<wstring, unsigned int> dict;
    vector<wstring> ordered_keys;
    if (argc!=3) exit(1);
    skipN = atol(argv[1]);
    topN = atol(argv[2]);
    ordered_keys.reserve(topN);
    setlocale(LC_ALL, "");

    n_features = read_topN_words("./.db/word_count/total.cnt", skipN, topN, read_buff, 255, ordered_keys, dict);
    assert(n_features==topN);
    assert(n_features==dict.size());
    assert(n_features==ordered_keys.size());

    fp = fopen("./.db/counts", "r");
    fscanf(fp, "%u", &n_records);
    fclose(fp); fp=NULL;

    fp = fopen("./.db/word_count/features.bin", "wb");
    fwrite(&n_records, sizeof(unsigned int), 1, fp);
    fwrite(&topN, sizeof(unsigned int), 1, fp);

    unsigned int percentage = 0, last_percentage = 0, done_workers = 0;
    for (unsigned int i=0; i<n_records; ++i) {
        unordered_map<wstring, unsigned int> local_map;
        unsigned int *feature = NULL;

        sprintf(fpath, "./.db/word_count/%d.cnt", i);
        local_map = read_histogram(fpath, read_buff, 255);

        feature = hist2vec(local_map, ordered_keys);
        fwrite(feature, sizeof(unsigned int), topN, fp);
        delete[] feature; feature=NULL;
        ++done_workers;
        percentage = (unsigned int)((float)(done_workers)/(float)n_records*100);
        if (percentage>last_percentage) {
            fprintf(stderr, "\rprogress: %3u%%", percentage);
            fflush(stderr);
            last_percentage = percentage;
        }
    }
    assert(done_workers==n_records);

    fclose(fp); fp=NULL;
    fprintf(stderr, "\nDone!\n");
    fflush(stderr);

    return 0;
}
