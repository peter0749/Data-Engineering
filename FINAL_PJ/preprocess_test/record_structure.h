#ifndef __INCLUDE_RECORD_STRUCTURE
#define __INCLUDE_RECORD_STRUCTURE
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

typedef struct __attribute__((__packed__)) {
    wchar_t url[64];
    wchar_t title[64];
    size_t  nbytes_of_content;
    wchar_t *content;
} news_record_structure;

void write_news_record(news_record_structure *news, const char *filepath);
void read_news_record(news_record_structure *news, const char *filepath);
void destroy_news_record(news_record_structure *news);

#endif
