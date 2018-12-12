#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "record_structure.h"

const char *file_name_prefix = "./data/ettoday";
const char *file_name_postfix = ".rec";
const size_t file_numbers = 6;
// const size_t file_numbers = 1;
const size_t file_prefix_length = 14;
const size_t buffer_limit = 131072;

// 中文 Unicode 區
const unsigned long min_chinese = 0x4E00;
const unsigned long max_chinese = 0x9FFF;
// End 中文 unicode 區

int8_t is_chinese(wchar_t c) {
    unsigned long c_ = (unsigned long)c;
    return ((c_>=min_chinese) && (c_<=max_chinese))?1:0;
}

void get_file_path(char *filename, size_t fno) {
    strcpy(filename, file_name_prefix);
    filename[file_prefix_length] = '0'+fno;
    strcpy(filename+file_prefix_length+1, file_name_postfix);
}

void format_line(wchar_t *str) { // 假設輸入 str 只有一行內容
    wchar_t *ptr = str;
    while(ptr!=NULL && *ptr!=0) {
        if (*ptr==L'\t' || *ptr==L'\b' || *ptr==L'\r') *ptr=L' '; // 清除空格與其他東西
        if (*ptr==L'\n') {  // 清除結尾換行符號
            *ptr=0;
            break;
        }
        ++ptr;
    }
}

unsigned long parse(void) {
    unsigned long record_counter=0;
    char filename[512];
    wchar_t *buffer = NULL, *ptr=NULL;
    FILE *fp = NULL;
    news_record_structure record;
    buffer = (wchar_t*)malloc(sizeof(wchar_t)*(buffer_limit+8));
    if (buffer==NULL) exit(10);
    for (size_t fno=0; fno<file_numbers; ++fno) {
        get_file_path(filename, fno);
        fp = fopen(filename, "rb");
        if (fp==NULL) return -1;

        while(fgetws(buffer, buffer_limit, fp)!=NULL) {
            if(wcsncmp(buffer, L"@GAISRec:", 9)!=0) continue; // 找下一筆資料的開頭
            memset(&record, 0x00, sizeof(record)); // zero-out record structure
            fgetws(buffer, buffer_limit, fp); // url
            format_line(buffer);
            wcsncpy(record.url, buffer+3, sizeof(record.url) / sizeof(record.url[0])-1);
            fgetws(buffer, buffer_limit, fp); // title
            format_line(buffer);
            wcsncpy(record.title, buffer+3, sizeof(record.title) / sizeof(record.title[0])-1);
            fgetws(buffer, buffer_limit, fp); // null
            fgetws(buffer, buffer_limit, fp); // context
            format_line(buffer);
            size_t nbytes = sizeof(wchar_t)*(wcslen(buffer)+1);
            record.nbytes_of_content = nbytes;
            record.content = buffer;
            sprintf(filename, "./.db/%lu.dat", record_counter);
            write_news_record(&record, filename);
            ++record_counter;
        }

        fclose(fp);
        fp = NULL;
    }
    free(buffer);
    fp = NULL;
    fp = fopen("./.db/counts", "w");
    if (fp==NULL) return -2;
    fprintf(fp, "%lu\n", record_counter);
    fclose(fp);
    fp = NULL;
    return record_counter;
}

int main(void) {
    setlocale(LC_ALL, ""); // 使用這個， fgetws 才不會出錯
    struct stat st = {0};
    if (stat("./.db", &st) == -1) {
        mkdir("./.db", 0700);
    }
    news_record_structure s;
    unsigned long news_n = parse();
    wprintf(L"Processed %lu news.\n", news_n);
    wprintf(L"test reading...\n");
    read_news_record(&s, "./.db/230656.dat");
    wprintf(L"url: %ls\ntitle: %ls\ncontent: %ls\n", s.url, s.title, s.content);
    destroy_news_record(&s);
    wprintf(L"done!\n");
    return 0;
}
