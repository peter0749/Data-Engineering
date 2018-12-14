#ifndef __INCLUDE_JIEBA_WORD_CNT__H
#define __INCLUDE_JIEBA_WORD_CNT__H
#include <iostream>
#include <string>
#include <unordered_map>
// #include <codecvt>
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

__attribute__((always_inline)) bool filter(const std::wstring &s) {
    if (s.length()<2 || s.length()>5) return false;
    for (const auto &v: s) {
        if(v<min_chinese || v>max_chinese) return false;
    }
    return true;
}

__attribute__((always_inline)) std::unordered_map<std::wstring, unsigned int> jieba_wordcount(const std::wstring &str, const cppjieba::MixSegment &jieba) {
    // static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::unordered_map<std::wstring, unsigned int> pattern;
    std::vector<std::string> words;
    // jieba.Cut(converter.to_bytes(str), words, true);
    jieba.Cut(WstringToString(str), words, true);
    for (const std::string &s : words) {
        // std::wstring ws(converter.from_bytes(s));
        std::wstring ws(StringToWstring(s));
        if (filter(ws)) {
            if (pattern.count(ws)==0) pattern.insert(std::make_pair<const std::wstring&, unsigned int>(ws, 1u));
            else ++pattern[ws];
        }
    }
    return pattern;
}

#endif
