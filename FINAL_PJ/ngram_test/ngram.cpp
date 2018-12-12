#include "ngram.h"
std::unordered_map<std::wstring, unsigned int> ngram_wordcount(std::wstring str, size_t N) {
    std::unordered_map<std::wstring, unsigned int> pattern;
    for (size_t i=0; i+N<=str.length(); ++i) {
        const std::wstring &s = str.substr(i, N);
        if (pattern.count(s)==0) pattern.insert(std::make_pair<const std::wstring&, unsigned int>(s, 1u));
        else ++pattern[s];
    }
    return pattern;
}
