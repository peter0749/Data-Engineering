#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <pthread.h>
#include <omp.h>
#include "jieba_word_count.hpp"

inline unsigned long read_article(const char *fpath, wchar_t *buffer, size_t buffer_limit) {
    wchar_t ch=WEOF;
    FILE *fp = fopen(fpath, "rb");
    unsigned long cnt=0;
    while(cnt<buffer_limit-1 && (ch = fgetwc(fp))!=WEOF) {
        buffer[cnt] = ch;
        ++cnt;
    }
    buffer[cnt] = 0;
    fclose(fp); fp=NULL;
    return cnt;
}

int main(void) {
    setlocale(LC_ALL, "");
    using namespace std;
    cppjieba::MixSegment jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
    vector<string> file_list;
    FILE *fp=NULL;
    struct stat st = {0};
    if (stat("./.db/word2vec", &st) == -1) {
        mkdir("./.db/word2vec", 0700);
    }

    /* Get File List */
    DIR *dp1=NULL, *dp2=NULL;
    struct dirent *dirp1=NULL;
    struct dirent *dirp2=NULL;
    dp1 = opendir("./wiki_data");
    while((dirp1 = readdir(dp1))!=NULL) {
        string subpath(dirp1->d_name);
        if (subpath=="."||subpath=="..") continue;
        subpath.insert(0, "./wiki_data/");
        dp2 = opendir(subpath.c_str());
        while((dirp2 = readdir(dp2))!=NULL) {
            string filename(dirp2->d_name);
            if (filename=="."||filename=="..") continue;
            filename = subpath + "/" + filename;
            file_list.push_back(filename);
        }
        closedir(dp2); dp2=NULL;
    }
    closedir(dp1); dp1=NULL;

    /*
    for (const auto &v: file_list) {
        cout << v << endl;
    }
    */

    fp = fopen("./.db/word2vec/corpus.txt", "ab");
    if (fp==NULL) exit(2);
    unsigned int file_n = file_list.size();
    #pragma omp parallel for shared(file_list,jieba,fp)
    for (unsigned long i=0; i<file_n; ++i) {
        char *fpath = new char[64];
        size_t article_lim = 81920;
        wchar_t *article = new wchar_t[article_lim];
        vector<wstring> words;
        #pragma omp critical(disk_io)
        {
            read_article(file_list[i].c_str(), article, article_lim-1);
        }
        words = tokenize_jieba(article, jieba);
        delete[] article; article=NULL;

        #pragma omp critical(disk_io)
        {
            for (const auto &s: words) {
                fputws(s.c_str(), fp);
                fputwc(L' ', fp);
            }
        }
        delete[] fpath; fpath = NULL;
    }
    fclose(fp); fp=NULL;

    return 0;
}
