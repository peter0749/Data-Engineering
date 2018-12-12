#include "ngram.h"
// 中文 Unicode 區
#define min_chinese 0x4E00
#define max_chinese 0x9FFF
// End 中文 unicode 區
inline bool in_range(std::wstring s) {
    for (auto v: s) {
        if(v<min_chinese || v>max_chinese) return false;
    }
    return true;
}
std::unordered_map<std::wstring, unsigned int> ngram_wordcount(std::wstring str, size_t N) {
    std::unordered_map<std::wstring, unsigned int> pattern;
    for (size_t i=0; i+N<=str.length(); ++i) {
        const std::wstring &s = str.substr(i, N);
        if (in_range(s)) {
            if (pattern.count(s)==0) pattern.insert(std::make_pair<const std::wstring&, unsigned int>(s, 1u));
            else ++pattern[s];
        }
    }
    return pattern;
}
