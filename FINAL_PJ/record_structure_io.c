#include "record_structure.h"

void write_news_record(news_record_structure *news, const char *filepath) {
    FILE *fp = NULL;
    fp = fopen(filepath, "wb");
    fwrite((void*)news->url, sizeof(wchar_t)*64, 1, fp);
    fwrite((void*)news->title, sizeof(wchar_t)*64, 1, fp);
    fwrite((void*)&news->nbytes_of_content, sizeof(size_t), 1, fp);
    fwrite((void*)news->content, news->nbytes_of_content, 1, fp);
    fclose(fp);
}

void read_news_record(news_record_structure *news, const char *filepath) {
    FILE *fp = NULL;
    fp = fopen(filepath, "rb");
    fread((void*)news->url, sizeof(wchar_t)*64, 1, fp);
    fread((void*)news->title, sizeof(wchar_t)*64, 1, fp);
    fread((void*)&news->nbytes_of_content, sizeof(size_t), 1, fp);
    news->content = (wchar_t*)malloc(news->nbytes_of_content);
    fread((void*)news->content, news->nbytes_of_content, 1, fp);
    fclose(fp);
}

void destroy_news_record(news_record_structure *news) {
    if(news->content!=NULL) free(news->content);
    news->content=NULL;
}
