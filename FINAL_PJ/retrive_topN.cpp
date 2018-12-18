#include <sys/types.h>
#include <sys/shm.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <locale>
#include <pthread.h>
#include <omp.h>
#include "jieba_word_count.hpp"
#include "read_histogram.hpp"
extern "C" double hist_intersection_d(unsigned int*, unsigned int*, unsigned int);

const size_t  read_buffer_size=8192;
wchar_t       read_buffer[read_buffer_size];

int main(int argc, char **argv) {
    using namespace std;
    int shm_id=0;
    unsigned int n_class=0;
    unsigned int n_rows=0, n_cols=0;
    unsigned int *_data=NULL;
    unsigned int **data=NULL;
    unsigned int *feature=NULL;
    double *distances=NULL;
    unsigned int *topN_id = NULL;
    int topN=1;
    FILE *fp = NULL;
    cppjieba::MixSegment jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
    unordered_map<wstring, unsigned int> word_cnt, class_map;
    priority_queue< pair<double,unsigned int>, vector< pair<double,unsigned int> >, less< pair<double,unsigned int> > > max_heap;
    setlocale(LC_ALL, "");
    if (argc<2) exit(4);
    topN = atol(argv[1]);
    if (argc>=3) freopen(argv[2], "rb", stdin);
    if (topN<1) topN=1;
    {
        wchar_t ch=0;
        size_t cnt=0;
#ifdef __APPLE__
        while ( (ch=fgetwc(stdin))!=EOF ) {
#else
        while ( (ch=fgetwc_unlocked(stdin))!=EOF ) {
#endif
            read_buffer[cnt++] = ch;
        }
        read_buffer[cnt] = 0;
    }

    fp = fopen("./ipc_addr", "r");
    if(fwscanf(fp, L"%d %d %d", &shm_id, &n_rows, &n_cols)!=3) {
        exit(1);
    }
    fclose(fp); fp=NULL;

    word_cnt = jieba_wordcount(wstring(read_buffer), jieba);
    class_map = get_histogram_mapping("./.db/word2vec/classes.txt", &n_class, read_buffer, read_buffer_size);
    feature = hist2vec(word_cnt, class_map, n_class); // from C

    _data = (unsigned int*)shmat(shm_id, NULL, 0);
    if (_data==(unsigned int*)-1) {
        perror("Error on shmat()");
        exit(2);
    }

    data = new unsigned int* [(long long)n_rows*n_cols];
    distances = new double[n_rows];
    topN_id = new unsigned int[n_rows];
    for (unsigned int i=0; i<n_rows; ++i) data[i] = _data+n_cols*i;
    #pragma omp parallel for shared(distances,feature,data) schedule(static,1)
    for (unsigned int i=0; i<n_rows; ++i) distances[i] = hist_intersection_d(feature, data[i], n_cols);

    for (int i=0; i<topN; ++i) max_heap.push({distances[i], i});
    for (unsigned int i=topN; i<n_rows; ++i) {
        if (distances[i]<max_heap.top().first) {
            max_heap.pop();
            max_heap.push({distances[i], i});
        }
    }

    for (int i=max_heap.size()-1; i>=0; --i) {
        topN_id[i] = max_heap.top().second;
        max_heap.pop();
    }
    for (int i=0; i<topN; ++i) fwprintf(stdout, L"%d %.2f\n", topN_id[i], distances[topN_id[i]]); 

    delete[] topN_id; topN_id=NULL;
    delete[] distances; distances=NULL;
    delete[] data; data=NULL;
    free(feature); feature=NULL;

    if (shmdt(_data)==-1) {
        perror("Error on shmdt()");
        exit(3);
    }

    return 0;
}
