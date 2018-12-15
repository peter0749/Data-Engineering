#ifndef __INCLUDE_JIEBA_WORD_CNT__H
#define __INCLUDE_JIEBA_WORD_CNT__H
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "cppjieba/include/cppjieba/MixSegment.hpp"
#include "wstringcvt.hpp"
#define DICT_PATH "cppjieba/dict/jieba.dict.utf8"
#define HMM_PATH "cppjieba/dict/hmm_model.utf8"
#define USER_DICT_PATH "cppjieba/dict/user.dict.utf8"
#define IDF_PATH "cppjieba/dict/idf.utf8"
#define STOP_WORD_PATH "cppjieba/dict/stop_words.utf8"

// 中文 Unicode 區
#define min_chinese 0x4E00
#define max_chinese 0x9FFF
// End 中文 unicode 區

__attribute__((always_inline)) std::vector<std::wstring> tokenize_jieba(wchar_t *wcs, const cppjieba::MixSegment &jieba) {
    const wchar_t *tokens = L"。？！\r\b\t\n?!";
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

__attribute__((always_inline)) std::unordered_map<std::wstring, unsigned int> jieba_wordcount(const std::wstring &str, const cppjieba::MixSegment &jieba) {
    std::unordered_map<std::wstring, unsigned int> pattern;
    std::vector<std::wstring> words;
    wchar_t *wcs = new wchar_t[str.length()+1];
    wcscpy(wcs, str.c_str());
    words = tokenize_jieba(wcs, jieba);
    for (const std::wstring &ws : words) {
        if (pattern.count(ws)==0) pattern.insert(std::make_pair<const std::wstring&, unsigned int>(ws, 1u));
        else ++pattern[ws];
    }
    delete[] wcs; wcs=NULL;
    return pattern;
}

#endif
