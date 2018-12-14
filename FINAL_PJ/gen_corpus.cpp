#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <omp.h>
#include "record_structure.h"
#include "jieba_word_count.hpp"
#include "wstringcvt.hpp"

int main(void) {
    setlocale(LC_ALL, "");
    using namespace std;
    const wchar_t *tokens = L"。？！\r\b\t\n?!";
    unsigned long n_records=0;
    cppjieba::MixSegment jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
    FILE *fp=NULL;
    struct stat st = {0};
    if (stat("./.db/word2vec", &st) == -1) {
        mkdir("./.db/word2vec", 0700);
    }

    fp = fopen("./.db/counts", "r");
    if (fp==NULL) exit(1);
    fscanf(fp, "%lu", &n_records);
    fclose(fp); fp=NULL;

    fp = fopen("./.db/word2vec/corpus.txt", "wb");
    if (fp==NULL) exit(2);
    #pragma omp parallel for shared(jieba,fp)
    for (unsigned long i=0; i<n_records; ++i) {
        char *fpath = NULL;
        news_record_structure *record=NULL;
        vector<wstring> words;
        record = new news_record_structure;
        fpath = new char[64];
        sprintf(fpath, "./.db/%lu.dat", i);
        #pragma omp critical(disk_io)
        {
            read_news_record(record, fpath);
        }
        words = tokenize_jieba(record->content, jieba);

        #pragma omp critical(disk_io)
        {
            for (const auto &s: words) {
                fputws(s.c_str(), fp);
                fputwc(L' ', fp);
            }
        }

        free(record->content); record->content=NULL;
        delete record; record=NULL;
    }
    fclose(fp); fp=NULL;

    return 0;
}
