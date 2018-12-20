#include <sys/types.h>
#include <sys/shm.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <locale>
#include <cmath>
#include <omp.h>
#include "flat_hash_map.hpp"
#include "cppjieba/include/cppjieba/MixSegment.hpp"
#include "wstringcvt.hpp"
#define DICT_PATH "cppjieba/dict/jieba.dict.utf8"
#define HMM_PATH "cppjieba/dict/hmm_model.utf8"
#define USER_DICT_PATH "cppjieba/dict/user.dict.utf8"

__attribute__((always_inline)) void get_histogram_mapping_inline(const char *fpath, ska::flat_hash_map<std::wstring, unsigned int> &word2class, wchar_t *buffer, size_t buffer_size) {
    FILE *fp=NULL;
    std::wstring read_line;
    std::wstring key;
    unsigned int label;
    fp = fopen(fpath, "rb");
    while(fgetws(buffer, buffer_size-1, fp)!=NULL) {
        std::wstringstream ss(buffer);
        ss>>key>>label;
        word2class.insert({key,label});
    }
    fclose(fp); fp=NULL;
}

__attribute__((always_inline)) unsigned int *hist2vec(const ska::flat_hash_map<std::wstring, unsigned int> &local_map, const ska::flat_hash_map<std::wstring, unsigned int> &key2class, unsigned int n_class) {
    using namespace std;
    unsigned int *feature = NULL;
    feature = new unsigned int[n_class];
    fill(feature, feature+n_class, 0); // initialize histogram feature
    for (const auto &f : local_map) {
        const auto iter = key2class.find(f.first);
        if (iter!=key2class.end()) {
            feature[ iter->second  ] += f.second;
        }
    }
    return feature;
}

__attribute__((always_inline)) std::vector<std::wstring> tokenize_jieba(wchar_t *wcs, const cppjieba::MixSegment &jieba) {
    const wchar_t *tokens = L"。？！，、：（）“”‘’‛‟.,‚″˝；「」\r\b\t\n?!《》【】╱;";
    wchar_t *ptr=NULL, *ptr2=NULL;
    std::vector<std::wstring> words;
    if (wcs==NULL || *wcs==0) return words;
    ptr = wcstok(wcs, tokens, &ptr2);
    while(ptr!=NULL) {
        std::vector<std::string> words_n;
        jieba.Cut(WstringToString(std::wstring(ptr)), words_n, true);
        for (const auto &s: words_n) words.push_back(StringToWstring(s));
        ptr = wcstok(NULL, tokens, &ptr2);
    }
    return words;
}

__attribute__((always_inline)) void jieba_wordcount_inplace(const std::wstring &str, const cppjieba::MixSegment &jieba, ska::flat_hash_map<std::wstring, unsigned int> &pattern) {
    std::vector<std::wstring> words;
    wchar_t *wcs = new wchar_t[str.length()+1];
    wcscpy(wcs, str.c_str());
    words = tokenize_jieba(wcs, jieba);
    for (const std::wstring &ws : words) {
        if (pattern.count(ws)==0) pattern.insert(std::make_pair<const std::wstring&, unsigned int>(ws, 1u));
        else ++pattern[ws];
    }
    delete[] wcs; wcs=NULL;
}

double hist_intersection(const float *P, const float *Q, unsigned int cols) {
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

double hist_intersection_normalized(const float *P, const float *Q, unsigned int cols) {
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
    const float *data=NULL;
    unsigned int *feature=NULL;
    float *feature_float=NULL;
    double *distances=NULL;
    unsigned int *topN_id = NULL;
    double (*D_func)(const float*, const float*, unsigned int)=NULL;
    int topN=1;
    char normalize=0;
    FILE *fp = NULL;
    cppjieba::MixSegment jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH);
    ska::flat_hash_map<wstring, unsigned int> word_cnt, class_map;
    priority_queue< pair<double,unsigned int>, vector< pair<double,unsigned int> >, less< pair<double,unsigned int> > > max_heap;
    setlocale(LC_ALL, "zh_TW.UTF-8");
    setlocale(LC_CTYPE, "zh_TW.UTF-8");
    if (argc<3) exit(4);
    topN = atol(argv[1]);
    normalize = argv[2][0]-'0';
    D_func = normalize?hist_intersection_normalized:hist_intersection;

    if (argc>=4) freopen(argv[3], "rb", stdin);
    class_map.reserve(700000);
    get_histogram_mapping_inline("./.db/word2vec/classes.txt", class_map, read_buffer, read_buffer_size);
    if (topN<1) topN=1;
    size_t cnt=0;
    {
        wchar_t ch=0;
        while ( (ch=fgetwc(stdin))!=WEOF ) 
        {
            read_buffer[cnt++] = ch;
        }
        read_buffer[cnt] = 0;
    }

    fp = fopen("./ipc_addr", "rb");
    if(fwscanf(fp, L"%d %d %d", &shm_id, &n_rows, &n_cols)!=3) {
        fwprintf(stdout, L"%d %d %d\n", shm_id, n_rows, n_cols);
        exit(1);
    }
    fclose(fp); fp=NULL;

    word_cnt.reserve(cnt+1);
    jieba_wordcount_inplace(wstring(read_buffer), jieba, word_cnt);
    feature = hist2vec(word_cnt, class_map, n_cols); // from C
    feature_float = new float[n_cols];
    for (unsigned int i=0; i<n_cols; ++i) feature_float[i] = (float)feature[i];

    data = (float*)shmat(shm_id, NULL, SHM_RDONLY);
    if (data==(float*)-1) {
        perror("Error on shmat()");
        exit(2);
    }

    distances = new double[n_rows];
    topN_id = new unsigned int[topN];

    #pragma omp parallel for simd shared(distances,feature_float,data)
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
