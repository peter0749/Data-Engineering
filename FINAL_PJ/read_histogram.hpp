#ifndef __INCLUDE_READ_HISTOGRAM__HPP
#define __INCLUDE_READ_HISTOGRAM__HPP
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdlib>
#include <cstdio>

inline std::unordered_map<std::wstring, unsigned int> get_histogram_mapping(const char *fpath, unsigned int *n_class, wchar_t *buffer, size_t buffer_size) {
    FILE *fp=NULL;
    std::wstring read_line;
    std::wstring key;
    std::unordered_set<unsigned int> s;
    std::unordered_map<std::wstring, unsigned int> word2class;
    unsigned int label;
    fp = fopen(fpath, "rb");
    while(fgetws(buffer, buffer_size-1, fp)!=NULL) {
        std::wstringstream ss(buffer);
        ss>>key>>label;
        word2class.insert({key,label});
        s.insert(label);
    }
    fclose(fp); fp=NULL;
    if (n_class!=NULL) *n_class = (unsigned int)s.size();
    return word2class;
}

inline unsigned int *hist2vec(const std::unordered_map<std::wstring, unsigned int> &local_map, const std::unordered_map<std::wstring, unsigned int> &key2class, unsigned int n_class) {
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

inline std::unordered_map<std::wstring, unsigned int> read_histogram(const char *fpath, wchar_t *read_buffer, size_t read_buffer_size) {
    using namespace std;
    FILE *fp=NULL;
    wstring key_s, sub;
    unsigned int value=0;
    unordered_map<wstring, unsigned int> dict;
    fp = fopen(fpath, "rb");
    while(fgetws(read_buffer, read_buffer_size-1, fp)!=NULL) {
        wstringstream ss(read_buffer);
        ss>>key_s>>value;
        dict.insert({key_s, value});
    }
    fclose(fp); fp=NULL;
    return dict;
}

#endif
