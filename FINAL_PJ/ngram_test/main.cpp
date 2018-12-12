#include <locale>
#include <iostream>
#include <string>
#include <unordered_map>
#include "ngram.h"

int main(void) {
    setlocale(LC_ALL, "");
    using namespace std;
    unsigned int N=2;
    wstring str;
    wcin >> N >> str;
    unordered_map<wstring, unsigned int> ngram(ngram_wordcount(str, N));
    for (auto v: ngram) {
        wcout << v.first << ' ' << v.second << endl;
    }
    return 0;
}
