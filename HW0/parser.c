#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include "postprocess.h"

const char *file_name_prefix = "./data/ettoday";
const char *temp_out_file = "./sentences.txt";
const char *final_out_file = "./dataset.txt";
const char *file_name_postfix = ".rec";
const size_t file_numbers = 6;
const size_t file_prefix_length = 14;
const size_t buffer_limit = 8192;
const size_t chinese_min_len = 6; // 一句話要有5個字以上（不包含）

// 中文 Unicode 區
const unsigned long min_chinese = 0x4E00;
const unsigned long max_chinese = 0x9FFF;
// End 中文 unicode 區

const wchar_t *tokens = L"。？！\r\b\t\n?!"; // 斷句 tokens

typedef struct {
    wchar_t *url; // 網頁 URL
    wchar_t *title; // 新聞標題
    wchar_t *context; // 新聞內文
} news_record;

int8_t is_chinese(wchar_t c) {
    unsigned long c_ = (unsigned long)c;
    return ((c_>=min_chinese) && (c_<=max_chinese))?1:0;
}

void get_file_path(char *filename, size_t fno) {
    strcpy(filename, file_name_prefix);
    filename[file_prefix_length] = '0'+fno;
    strcpy(filename+file_prefix_length+1, file_name_postfix);
}

int tokenize(wchar_t ***results, wchar_t *str) {
    wchar_t **sentences=NULL, **s_ptr_t=NULL;
    wchar_t *ptr=NULL, *buff=NULL;
    int cnt = 0;
    int cap = 32; // 初始容量, 三十二個寬字元指標
    sentences = (wchar_t**) malloc(sizeof(wchar_t*)*cap);
    if (sentences==NULL) exit(1);
    ptr = wcstok(str, tokens, &buff);
    while(ptr!=NULL) {
        unsigned long ch_cnt = 0;
        wchar_t *ch_test_ptr = ptr;
        if (ch_test_ptr!=NULL && is_chinese(ch_test_ptr[0])) { // 第一個字必須是中文字
            while(ch_test_ptr!=NULL && *ch_test_ptr!=0) {
                ch_cnt += is_chinese(*ch_test_ptr);
                ++ch_test_ptr;
            }
        }
        if (ch_cnt>=chinese_min_len) { // 一句中文必須達到6個字(含)以上
            if(cnt==cap) { // buffer 滿了
                cap *= 2; // 增加一倍容量
                s_ptr_t = NULL;
                s_ptr_t = (wchar_t**) malloc(sizeof(wchar_t*)*cap);
                if (s_ptr_t==NULL) exit(2);
                memcpy(s_ptr_t, sentences, sizeof(wchar_t*)*cnt);
                free(sentences);
                sentences = s_ptr_t;
                s_ptr_t = NULL;
            }
            sentences[cnt] = NULL; 
            sentences[cnt] = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(ptr)+1));
            if (sentences[cnt]==NULL) exit(4);
            wcscpy(sentences[cnt], ptr);
            ++cnt;
        }
        ptr = wcstok(NULL, tokens, &buff);
    }
    s_ptr_t = NULL;
    s_ptr_t = (wchar_t**) malloc(sizeof(wchar_t*)*cnt);
    if (s_ptr_t==NULL) exit(3);
    memcpy(s_ptr_t, sentences, sizeof(wchar_t*)*cnt);
    free(sentences);
    sentences = s_ptr_t;
    s_ptr_t = NULL;
    *results = sentences;
    return cnt;
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

int parse(void) {
    char filename[32];
    wchar_t *buffer = NULL, *ptr=NULL;
    wchar_t **sentences = NULL;
    FILE *fp = NULL;
    FILE *fout = NULL;
    news_record one_record;
    fout = fopen(temp_out_file, "wb");
    if (fout==NULL) exit(9);
    buffer = (wchar_t*)malloc(sizeof(wchar_t)*(buffer_limit+8));
    if (buffer==NULL) exit(10);
    for (size_t fno=0; fno<file_numbers; ++fno) {
        get_file_path(filename, fno);
        fp = fopen(filename, "rb");
        if (fp==NULL) return -1;

        while(fgetws(buffer, buffer_limit, fp)!=NULL) {
            if(wcsncmp(buffer, L"@GAISRec:", 9)!=0) continue; // 找下一筆資料的開頭
            fgetws(buffer, buffer_limit, fp); // url
            format_line(buffer);
            one_record.url = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(buffer)+1));
            wcscpy(one_record.url, buffer+3);
            fgetws(buffer, buffer_limit, fp); // title
            format_line(buffer);
            one_record.title = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(buffer)+1));
            wcscpy(one_record.title, buffer+3);
            fgetws(buffer, buffer_limit, fp); // null
            fgetws(buffer, buffer_limit, fp); // context
            one_record.context = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(buffer)+1));
            wcscpy(one_record.context, buffer);
            int s_cnt = 0;
            s_cnt = tokenize(&sentences, one_record.context); // 這裡做斷句
            free(one_record.context); one_record.context=NULL;
            for (size_t i=0; i<s_cnt; ++i) {
                fwprintf(fout, L"%ls\t%ls\t%ls\n", sentences[i], one_record.title, one_record.url); // 寫入句子到檔案
                free(sentences[i]); sentences[i]=NULL;
            }
            free(sentences); sentences=NULL;
            free(one_record.url); one_record.url=NULL;
            free(one_record.title); one_record.title=NULL;
        }

        fclose(fp);
        fp = NULL;
    }
    free(buffer);
    fclose(fout);
    return 0;
}

void read_and_sort(void) {
    FILE *fp = NULL, *fout=NULL;
    wchar_t **sentences = NULL, *buffer=NULL, **new_s_p=NULL, *new_row=NULL;
    size_t cap = 1024;
    fp = fopen(temp_out_file, "rb");
    fout = fopen(final_out_file, "wb");
    sentences = (wchar_t**) malloc(sizeof(wchar_t*)*cap);
    if (sentences==NULL) exit(5);
    buffer = (wchar_t*) malloc(sizeof(wchar_t)*(buffer_limit+8));
    if (buffer==NULL) exit(6);

    size_t cnt = 0;
    while(fgetws(buffer, buffer_limit, fp)!=NULL) {
        if (cnt==cap) {
            cap*=2;
            new_s_p = NULL;
            new_s_p = (wchar_t**)malloc(sizeof(wchar_t*)*cap);
            if (new_s_p==NULL) exit(7);
            memcpy(new_s_p, sentences, sizeof(wchar_t*)*cnt);
            free(sentences);
            sentences = new_s_p;
            new_s_p = NULL;
        }
        size_t len = wcslen(buffer);
        new_row = NULL;
        new_row = (wchar_t*)malloc(sizeof(wchar_t)*(len+1));
        if (new_row==NULL) exit(8);
        wcscpy(new_row, buffer);
        sentences[cnt] = new_row;
        new_row = NULL;
        ++cnt;
    }
    fclose(fp); fp=NULL;
    free(buffer); buffer=NULL;

    postprocess(sentences, cnt); // sort in lexical order

    for (size_t i=0; i<cnt; ++i) { // traverse sorted sentences
        fputws(sentences[i], fout);
        free(sentences[i]);
        sentences[i] = NULL;
    }
    free(sentences);
    fclose(fout); fout=NULL;
}

int main(void) {
    setlocale(LC_ALL, ""); // 使用這個， fgetws 才不會出錯
    parse();
    read_and_sort();
    return 0;
}
