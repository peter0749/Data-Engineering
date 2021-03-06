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
#include "filereader.hpp"

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

std::string copy_append_encoder(const unsigned char *fileA, const unsigned char *fileB, \
                         int A_len, int B_len, unsigned int K_size) {
    using namespace std;
    int B_index=0;
    ska::flat_hash_map<string, vector<int> > kgramA;
    string delta;
    int *z_buffer = new int[A_len+B_len+5];

    /* Generate Kgram index for fileA */
    for (int i=0; i<A_len; ++i) {
        int l = min((int)(i+K_size), A_len) - i;
        string ss((char*)(fileA+i), l);
        if(kgramA.count(ss)==0) kgramA.insert({ss, vector<int>(1, i)});
        else kgramA[ss].push_back(i);
    }
    
    string lastKgramB;
    int last_msg_length = 0;

    while (B_index<B_len) {
        int l = min((int)(B_index+K_size), B_len) - B_index;
        int m;
        string kgramB((char*)(fileB+B_index), l);
        if (kgramA.count(kgramB)==0) {
            string append_msg; 
            if (last_msg_length>0) delta.resize(delta.length()-last_msg_length);
            lastKgramB += kgramB;
            append_msg += "a " + to_string(lastKgramB.length());
            append_msg += "\n" + lastKgramB;
            delta += append_msg;
            last_msg_length = append_msg.length();
        } else {
            /* Find longest match between A and B[B_index:] */
            pair<int,int> ML = find_longest_match(fileA, kgramA[kgramB], fileB+B_index, A_len, B_len-B_index, z_buffer);
            m = ML.first;
            l = ML.second;
            string copy_msg;
            copy_msg += "c " + to_string(m);
            copy_msg += ","  + to_string(l);
            copy_msg += "\n";
            delta += copy_msg;
            lastKgramB.clear();
            last_msg_length = 0;
        }
        B_index += l;
    }
    lastKgramB.clear();
    if (z_buffer!=NULL) delete[] z_buffer; z_buffer=NULL;
    return delta;
}

int main(int argc, char **argv) {
    using namespace std;
    int  *SA=NULL;
    unsigned char *fileA=NULL, *fileB=NULL;
    int fileA_len=0, fileB_len=0;
    int K_size=5;
    string delta;
    if (argc!=4) {
        fprintf(stderr, "./copy_append_model fileA fileB K_size\n");
        exit(1);
    }
    
    fileA_len = open_n_read_file(argv[1], &fileA);
    fileB_len = open_n_read_file(argv[2], &fileB);
    K_size = atoi(argv[3]);

    delta = copy_append_encoder(fileA, fileB, fileA_len, fileB_len, K_size);
    fwrite(delta.c_str(), sizeof(char), delta.length(), stdout);

    if (fileA!=NULL) delete[] fileA; fileA=NULL;
    if (fileB!=NULL) delete[] fileB; fileB=NULL;

    return 0;
}
