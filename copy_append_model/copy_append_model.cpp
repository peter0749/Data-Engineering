#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <climits>
#include <algorithm>
#include <pthread.h>
#include <omp.h>
#include "flat_hash_map.hpp"
#include "SegmentTree.hpp"
#include "filereader.hpp"

extern "C" int sais(const unsigned char*, int*, int);

std::pair<int,int> find_longest_match(int *sa, int *rank, SegmentTree *lcpa_tree, unsigned char *str, int N, int offset, int A_len) {
    /* Find longest match between A and B. 
     * That is arg_{L,M}_max_{L} { A[M:M+L] == B[0:L] for all A and B in range }
     * A: 0~offset-1, B: offset~N-1 */
    using namespace std;
    int M=0, L=0;
    int B_srt_sa_pos = rank[offset];

    #pragma omp parallel for shared(rank, B_srt_sa_pos, lcpa_tree, L, M) schedule(dynamic)
    for (int m=0; m<A_len; ++m) {
        int A_srt_sa_pos = rank[m];
        int s = min(A_srt_sa_pos, B_srt_sa_pos);
        int t = max(A_srt_sa_pos, B_srt_sa_pos);
        // if (s+1>t) continue; // illigal
        int l = (s+1>t)?INT_MAX:lcpa_tree->query(s+1, t); // [s+1, t]
        #pragma omp critical(update_L)
        {
            if (l>L) {
                L = l;
                M = m;
            }
        }
    }

    return {M,L};
}

void copy_append_encoder(const unsigned char *fileA, const unsigned char *fileB, \
                         int A_len, int B_len, unsigned int K_size) {
    using namespace std;
    unsigned char *longest_match_buffer=NULL;
    int B_index=0;
    ska::flat_hash_map<string, int> kgramA;
    unsigned char *catAB = NULL;
    int *sa = NULL, *lcpa = NULL, *rank = NULL;
    SegmentTree *lcpa_tree=NULL;
    catAB = new unsigned char[B_len+A_len+5];
    memcpy(catAB,       fileA, A_len*sizeof(unsigned char));
    memcpy(catAB+A_len, fileB, B_len*sizeof(unsigned char));
    sa = new int[B_len+A_len+5];

    /* Create suffix array */
    sais(catAB, sa, B_len+A_len);
    lcpa = new int[B_len+A_len+5];
    rank = new int[B_len+A_len+5];

    /* Create rank array (inverse of suffix array) */
    for (int i=0; i<A_len+B_len; ++i) rank[sa[i]] = i;

    /* Create longest common prefix array */
    for (int i=0, lcp=0; i<A_len+B_len; ++i) {
        if (rank[i]==0) lcpa[0]=0;
        else {
            int j = sa[rank[i]-1];
            if (lcp>0) --lcp;
            while (catAB[i+lcp]==catAB[j+lcp]) ++lcp;
            lcpa[rank[i]] = lcp;
        }
    }

    /* Create Segment Tree */
    lcpa_tree = new SegmentTree(lcpa, A_len+B_len);
    if (lcpa!=NULL) delete[] lcpa; lcpa=NULL;

    /* Generate Kgram index for fileA */
    for (int i=0; i<A_len; ++i) {
        int l = min((int)(i+K_size), A_len) - i;
        kgramA.insert({string((char*)fileA, i, l), i});
    }
    
    while (B_index<B_len) {
        int l = min((int)(B_index+K_size), B_len) - B_index;
        int m;
        string kgramB((char*)fileB, B_index, l);
        if (kgramA.count(kgramB)==0) {
            string append_msg; 
            append_msg += "a " + to_string(l);
            append_msg += "\n" + kgramB;
            fwrite(append_msg.c_str(), sizeof(char), append_msg.length(), stdout);
        } else {
            /* Find longest match between A and B[B_index:] */
            pair<int,int> ML = find_longest_match(sa,rank,lcpa_tree,catAB,A_len+B_len,A_len+B_index,A_len);
            m = ML.first;
            l = ML.second;
            string copy_msg;
            copy_msg += "c " + to_string(m);
            copy_msg += ","  + to_string(l);
            copy_msg += "\n";
            fwrite(copy_msg.c_str(), sizeof(char), copy_msg.length(), stdout);
        }
        B_index += l;
    }
    if (lcpa_tree!=NULL) delete lcpa_tree; lcpa_tree=NULL;
    if (catAB!=NULL) delete[] catAB; catAB=NULL;
    if (sa!=NULL) delete[] sa; sa=NULL;
    if (rank!=NULL) delete[] rank; rank=NULL;
}

int main(int argc, char **argv) {
    using namespace std;
    int  *SA=NULL;
    unsigned char *fileA=NULL, *fileB=NULL;
    int fileA_len=0, fileB_len=0;
    int K_size=5;
    if (argc!=4) {
        fprintf(stderr, "./copy_append_model fileA fileB K_size\n");
        exit(1);
    }
    
    fileA_len = open_n_read_file(argv[1], &fileA);
    fileB_len = open_n_read_file(argv[2], &fileB);
    K_size = atoi(argv[3]);

    copy_append_encoder(fileA, fileB, fileA_len, fileB_len, K_size);

    if (fileA!=NULL) delete[] fileA; fileA=NULL;
    if (fileB!=NULL) delete[] fileB; fileB=NULL;

    return 0;
}
