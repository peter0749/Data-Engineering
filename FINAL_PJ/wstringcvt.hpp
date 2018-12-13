#ifndef __INCLUDE_WSTRINGCVT_H_H
#define __INCLUDE_WSTRINGCVT_H_H
#include <string>
#include <locale.h> 

__attribute__((always_inline)) std::wstring StringToWstring(const std::string &str) {
    unsigned len = str.size() << 1;
    setlocale(LC_CTYPE, "");
    wchar_t *p = new wchar_t[len];
    mbstowcs(p,str.c_str(),len);
    std::wstring str1(p);
    delete[] p;
    return str1;
}

__attribute__((always_inline)) std::string WstringToString(const std::wstring &str) {
    unsigned len = str.size() << 2;
    setlocale(LC_CTYPE, "");
    char *p = new char[len];
    wcstombs(p,str.c_str(),len);
    std::string str1(p);
    delete[] p;
    return str1;
}

#endif
