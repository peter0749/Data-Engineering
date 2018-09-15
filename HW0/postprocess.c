#include "postprocess.h"

int wchar_cmp_func(const void *a, const void *b) {
    wchar_t **c = (wchar_t**)a;
    wchar_t **d = (wchar_t**)b;
    return wcscmp(*c, *d);
}

void postprocess(wchar_t **str, size_t cnt) {
    qsort((void*)str, cnt, sizeof(wchar_t*), wchar_cmp_func);
}

