#ifndef __INCLUDE_NGRAM_H
#define __INCLUDE_NGRAM_H
#include <iostream>
#include <string>
#include <unordered_map>
std::unordered_map<std::wstring, unsigned int> ngram_wordcount(std::wstring str, size_t N);
#endif
