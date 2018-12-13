#ifndef __INCLUDE_READ_HISTOGRAM__HPP
#define __INCLUDE_READ_HISTOGRAM__HPP
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <cstdio>

inline unsigned int *hist2vec(const std::unordered_map<std::wstring, unsigned int> &local_map, const std::vector<std::wstring> &ordered_keyset) {
    using namespace std;
    unsigned int *feature = NULL;
    unsigned int fn = 0;
    unsigned int topN = ordered_keyset.size();
    feature = new unsigned int[topN];
    fill(feature, feature+topN, 0); // initialize histogram feature
    fn = 0;
    for (const auto &f : ordered_keyset) {
        const auto iter = local_map.find(f);
        if (iter!=local_map.end()) feature[fn] = iter->second;
        ++fn;
    }
    return feature;
}

inline unsigned int read_topN_words(const char *fpath, unsigned int skipN, unsigned int topN, wchar_t *read_buffer, size_t read_buffer_size,\
        std::vector<std::wstring> &ordered_keys, std::unordered_map<std::wstring, unsigned int> &dict) {
    using namespace std;
    unsigned int n_features=0;
    FILE *fp = NULL;
    wstring key_s;
    fp = fopen(fpath, "rb");
    while(skipN-- && fgetws(read_buffer, read_buffer_size-1, fp)!=NULL); // skip first N words
    while(n_features<topN && fgetws(read_buffer, read_buffer_size-1, fp)!=NULL) {
        wstringstream ss(read_buffer);
        getline(ss, key_s, L',');
        dict.insert({key_s, n_features});
        ordered_keys.push_back(key_s);
        ++n_features;
    }
    fclose(fp); fp=NULL;
    return n_features;
}

inline std::unordered_map<std::wstring, unsigned int> read_histogram(const char *fpath, wchar_t *read_buffer, size_t read_buffer_size) {
    using namespace std;
    FILE *fp=NULL;
    wstring key_s, sub;
    unsigned int value=0;
    unordered_map<wstring, unsigned int> dict;
    fp = fopen(fpath, "rb");
    while(fgetws(read_buffer, read_buffer_size-1, fp)!=NULL) {
        wstringstream ss(read_buffer);
        getline(ss, key_s, L',');
        getline(ss, sub, L',');
        value = stoul(sub);
        dict.insert({key_s, value});
    }
    fclose(fp); fp=NULL;
    return dict;
}

#endif
