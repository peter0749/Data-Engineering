#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "record_structure.h"
#include "ngram.h"

template<class A, class B>
bool sortBySec(const std::pair<A,B>&a, const std::pair<A,B>&b) {
    return a.second > b.second;
}

int main(void) {
    setlocale(LC_ALL, "");
    using namespace std;
    char str[512];
    unsigned long n_records=0;
    unordered_map<wstring, unsigned int> *word_cnt_tot = new unordered_map<wstring, unsigned int>();
    unordered_map<wstring, unsigned int> *word_cnt_local=NULL;
    vector< pair<wstring, unsigned int> > *v=NULL;
    FILE *fp=NULL;
    struct stat st = {0};
    news_record_structure record;
    if (stat("./.db/word_count", &st) == -1) {
        mkdir("./.db/word_count", 0700);
    }

    fp = fopen("./.db/counts", "r");
    if (fp==NULL) exit(1);
    fscanf(fp, "%lu", &n_records);
    fclose(fp);

    for (unsigned long i=0; i<n_records; ++i) {
        sprintf(str, "./.db/%lu.dat", i);
        read_news_record(&record, str);
        word_cnt_local = new unordered_map<wstring, unsigned int>(ngram_wordcount(wstring(record.content), 2));
        v = new vector< pair<wstring, unsigned int> >(word_cnt_local->begin(), word_cnt_local->end());
        delete word_cnt_local; word_cnt_local=NULL;
        sort(v->begin(), v->end(), sortBySec<wstring, unsigned int>);
        sprintf(str, "./.db/word_count/%lu.cnt", i);
        fp = NULL;
        fp = fopen(str, "wb");
        if (fp==NULL) exit(2);
        // write to file
        for (auto e: *v) {
            fwprintf(fp, L"%ls,%u\n", e.first.c_str(), e.second);
        }
        // update global word count
        for (auto e: *v) {
            if (word_cnt_tot->count(e.first)==0) word_cnt_tot->insert(e);
            else (*word_cnt_tot)[e.first]+=e.second;
        }
        fclose(fp);
        fp=NULL;
        delete v; v=NULL;
    }

    v = new vector< pair<wstring, unsigned int> >(word_cnt_tot->begin(), word_cnt_tot->end());
    delete word_cnt_tot; word_cnt_tot=NULL;
    sort(v->begin(), v->end(), sortBySec<wstring, unsigned int>);
    fp = NULL;
    fp = fopen("./.db/word_count/total.cnt", "wb");
    if (fp==NULL) exit(3);
    // write to file
    for (auto e: *v) {
        fwprintf(fp, L"%ls,%u\n", e.first.c_str(), e.second);
    }
    delete v; v=NULL;
    return 0;
}
