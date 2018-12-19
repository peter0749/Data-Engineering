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

double hist_intersection(float *P, float *Q, unsigned int cols) {
    double P_M = 0.0;
    double Q_M = 0.0;
    double JSD = 0.0;
    for (unsigned int i=0; i<cols; ++i) {
        float M_i = (P[i]+Q[i]) / 2.0 + 1e-6;
        P_M += P[i]*log((P[i]+1e-6) / M_i);
        Q_M += Q[i]*log((Q[i]+1e-6) / M_i);
    }
    JSD = (P_M+Q_M) / 2.0;
    return JSD*JSD;
}

double hist_intersection_normalized(float *P, float *Q, unsigned int cols) {
    double P_M = 0.0;
    double Q_M = 0.0;
    double JSD = 0.0;
    double P_S = 1e-6;
    double Q_S = 1e-6;
    for (unsigned int i=0; i<cols; ++i) {
        P_S += P[i];
        Q_S += Q[i];
    }
    for (unsigned int i=0; i<cols; ++i) {
        float p=P[i]/P_S, q=Q[i]/Q_S;
        float M_i = (p+q) / 2.0 + 1e-6;
        P_M += p*log((p+1e-6) / M_i);
        Q_M += q*log((q+1e-6) / M_i);
    }
    JSD = (P_M+Q_M) / 2.0;
    return JSD*JSD;
}

const size_t  read_buffer_size=8192;
wchar_t       read_buffer[read_buffer_size];

int main(int argc, char **argv) {
    using namespace std;
    int shm_id=0;
    unsigned int n_rows=0, n_cols=0;
    float *data=NULL;
    unsigned int *feature=NULL;
    float *feature_float=NULL;
    double *distances=NULL;
    unsigned int *topN_id = NULL;
    double (*D_func)(float*, float*, unsigned int)=NULL;
    int topN=1;
    char normalize=0;
    FILE *fp = NULL;
    cppjieba::MixSegment jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
    unordered_map<wstring, unsigned int> word_cnt, class_map;
    priority_queue< pair<double,unsigned int>, vector< pair<double,unsigned int> >, less< pair<double,unsigned int> > > max_heap;
    setlocale(LC_ALL, "");
    if (argc<3) exit(4);
    topN = atol(argv[1]);
    normalize = argv[2][0]-'0';
    D_func = normalize?hist_intersection_normalized:hist_intersection;

    if (argc>=4) freopen(argv[3], "rb", stdin);
    class_map = get_histogram_mapping("./.db/word2vec/classes.txt", NULL, read_buffer, read_buffer_size);
    if (topN<1) topN=1;
    size_t cnt=0;
    {
        wchar_t ch=0;
#ifdef __APPLE__
        while ( (ch=fgetwc(stdin))!=EOF ) 
#else
        while ( (ch=fgetwc_unlocked(stdin))!=EOF ) 
#endif
        {
            read_buffer[cnt++] = ch;
        }
        read_buffer[cnt] = 0;
    }

    fp = fopen("./ipc_addr", "r");
    if(fwscanf(fp, L"%d %d %d", &shm_id, &n_rows, &n_cols)!=3) {
        exit(1);
    }
    fclose(fp); fp=NULL;

    word_cnt.reserve(cnt+1);
    jieba_wordcount_inplace(wstring(read_buffer), jieba, word_cnt);
    feature = hist2vec(word_cnt, class_map, n_cols); // from C
    feature_float = new float[n_cols];
    for (unsigned int i=0; i<n_cols; ++i) feature_float[i] = (float)feature[i];

    data = (float*)shmat(shm_id, NULL, 0);
    if (data==(float*)-1) {
        perror("Error on shmat()");
        exit(2);
    }

    distances = new double[n_rows];
    topN_id = new unsigned int[topN];

    #pragma omp parallel for shared(distances,feature,data) schedule(static,1)
    for (unsigned int i=0; i<n_rows; ++i) distances[i] = D_func(feature_float, data+n_cols*i, n_cols);

    for (int i=0; i<topN; ++i) max_heap.push({distances[i], i});
    for (unsigned int i=topN; i<n_rows; ++i) {
        if (distances[i]<max_heap.top().first) {
            max_heap.pop();
            max_heap.push({distances[i], i});
        }
    }

    topN = max_heap.size();
    for (int i=topN-1; i>=0; --i) {
        topN_id[i] = max_heap.top().second;
        max_heap.pop();
    }
    for (int i=0; i<topN; ++i) fwprintf(stdout, L"%d %.2f\n", topN_id[i], distances[topN_id[i]]); 

    delete[] topN_id; topN_id=NULL;
    delete[] distances; distances=NULL;
    delete[] feature_float; feature_float=NULL;
    free(feature); feature=NULL;

    if (shmdt(data)==-1) {
        perror("Error on shmdt()");
        exit(3);
    }

    return 0;
}
