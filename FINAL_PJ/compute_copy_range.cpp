#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <climits>
#include <algorithm>
#include <vector>
#include "flat_hash_map.hpp"

inline std::pair<int,int> find_longest_match(const unsigned char *fileA, const std::vector<int> &shortcut_index, const unsigned char *fileB, int A_len, int B_len, int *z) {
    using std::min;
    using std::max;
    fileA += shortcut_index[0];
    A_len -= shortcut_index[0];
    memset(z, 0x00, sizeof(int)*(A_len+B_len));
#define s(i) (i<B_len?fileB[i]:fileA[i-B_len])
    // From Eddy's codebook:
    int l=0, r=0;
    z[0]=A_len+B_len;
    for (int i=1; i<A_len+B_len; ++i) {
        int j = max(min(z[i-l],r-i),0);
        while(i+j<A_len+B_len&&s(i+j)==s(j)) ++j;
        z[i] = j;
        if (i+z[i]>r) r=i+z[i], l=i;
    }
#undef s
    int M=B_len;
    int L=z[M];
    for (auto v : shortcut_index) {
        int m = v+B_len-shortcut_index[0];
        int l = z[m];
        if (l>L) {
            L = l;
            M = m;
        }
    }
    if (L>B_len) L = B_len; // Although this happen, the value of M is correct.
    M = M-B_len+shortcut_index[0];
    return {M,L};
}

std::vector< std::pair<int,int> > copy_detect(const unsigned char *fileA, const unsigned char *fileB, \
                         int A_len, int B_len, unsigned int K_size) {
    using namespace std;
    int B_index=0;
    ska::flat_hash_map<string, vector<int> > kgramA;
    vector< pair<int,int> > ret;
    int *z_buffer = new int[A_len+B_len+5];

    /* Generate Kgram index for fileA */
    for (int i=0; i<A_len; ++i) {
        int l = min((int)(i+K_size), A_len) - i;
        string ss((char*)(fileA+i), l);
        if(kgramA.count(ss)==0) kgramA.insert({ss, vector<int>(1, i)});
        else kgramA[ss].push_back(i);
    }
    
    while (B_index<B_len) {
        int l = min((int)(B_index+K_size), B_len) - B_index;
        int m;
        string kgramB((char*)(fileB+B_index), l);
        if (kgramA.count(kgramB)>0) {
            /* Find longest match between A and B[B_index:] */
            pair<int,int> ML = find_longest_match(fileA, kgramA[kgramB], fileB+B_index, A_len, B_len-B_index, z_buffer);
            m = ML.first;
            l = ML.second;
            ret.push_back({m,m+l}); // [, )
        }
        B_index += l;
    }
    if (z_buffer!=NULL) delete[] z_buffer; z_buffer=NULL;
    return ret;
}

inline std::vector< std::pair<int,int> > summary_ranges(std::vector< std::pair<int,int> > &ranges) {
    using namespace std;
    vector< pair<int,int> > ret;
    sort(ranges.begin(), ranges.end());
    ranges.push_back({INT_MAX,INT_MAX}); // 哨兵
    int l=ranges[0].first, r=ranges[0].second;
    for (int i=1; i<ranges.size(); ++i) {
        if (ranges[i].first<=r) r = max(r, ranges[i].second);
        else {
            ret.push_back({l,r});
            l = ranges[i].first;
            r = ranges[i].second;
        }
    }
    ranges.pop_back();
    return ret;
}

int main(int argc, char **argv) {
    using namespace std;
    unsigned char *fileA=NULL, *fileB=NULL;
    int fileA_len=0, fileB_len=0;
    const int K_size=12;
    vector< pair<int,int> > copy_ranges;
    
    fscanf(stdin, "%d %d\n", &fileA_len, &fileB_len);
    fileA = new unsigned char[fileA_len+5];
    fileB = new unsigned char[fileB_len+5];
    
    fread(fileA, 1, fileA_len, stdin);
    fread(fileB, 1, fileB_len, stdin);

    copy_ranges = copy_detect(fileA, fileB, fileA_len, fileB_len, K_size);
    copy_ranges = summary_ranges(copy_ranges);

    for (auto v : copy_ranges) {
        fprintf(stdout, "%d %d\n", v.first, v.second);
    }

    if (fileA!=NULL) delete[] fileA; fileA=NULL;
    if (fileB!=NULL) delete[] fileB; fileB=NULL;

    return 0;
}
