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
#include <pthread.h>
#include <omp.h>
#include "record_structure.h"
#include "jieba_word_count.hpp"

template<class A, class B>
bool sortBySec(const std::pair<A,B>&a, const std::pair<A,B>&b) {
    return a.second > b.second;
}

int main(void) {
    setlocale(LC_ALL, "");
    using namespace std;
    unsigned long n_records=0;
    unordered_map<wstring, unsigned int> *word_cnt_tot = new unordered_map<wstring, unsigned int>();
    vector< pair<wstring, unsigned int> > *v=NULL;
    FILE *fp=NULL;
    struct stat st = {0};
    cppjieba::Jieba jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
    if (stat("./.db/word_count", &st) == -1) {
        mkdir("./.db/word_count", 0700);
    }

    fp = fopen("./.db/counts", "r");
    if (fp==NULL) exit(1);
    fscanf(fp, "%lu", &n_records);
    fclose(fp);

    #pragma omp parallel for shared(jieba,word_cnt_tot)
    for (unsigned long i=0; i<n_records; ++i) {
        char *fpath = NULL;
        FILE *foutp = NULL;
        news_record_structure *record=NULL;
        unordered_map<wstring, unsigned int> *word_cnt_local=NULL;
        vector< pair<wstring, unsigned int> > *v_local=NULL;
        record = new news_record_structure;
        fpath = new char[64];
        sprintf(fpath, "./.db/%lu.dat", i);
        #pragma omp critical(disk_io_section)
        {
            read_news_record(record, fpath);
        }
        word_cnt_local = new unordered_map<wstring, unsigned int>(jieba_wordcount(wstring(record->content), jieba));
        delete record; record=NULL;
        v_local = new vector< pair<wstring, unsigned int> >(word_cnt_local->begin(), word_cnt_local->end());
        delete word_cnt_local; word_cnt_local=NULL;
        sort(v_local->begin(), v_local->end(), sortBySec<wstring, unsigned int>);
        sprintf(fpath, "./.db/word_count/%lu.cnt", i);
        foutp = NULL;
        #pragma omp critical(disk_io_section)
        {
            foutp = fopen(fpath, "wb");
            // write to file
            for (auto e: *v_local) {
                fwprintf(foutp, L"%ls,%u\n", e.first.c_str(), e.second);
            }
            fclose(foutp);
            foutp=NULL;
        }
        delete [] fpath;

        // update global word count (critical section)
        #pragma omp critical(map_update)
        {
            for (auto e: *v_local) {
                if (word_cnt_tot->count(e.first)==0) word_cnt_tot->insert(e);
                else (*word_cnt_tot)[e.first]+=e.second;
            }
        }
        delete v_local; v_local=NULL;
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
