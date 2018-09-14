#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>

const char *file_name_prefix = "./data/ettoday";
const char *file_name_postfix = ".rec";
const size_t file_numbers = 6;
const size_t file_prefix_length = 14;
const size_t buffer_limit = 8192;
const size_t chinese_min_len = 6;

// 中文 Unicode 區
const unsigned long min_chinese = 0x4E00;
const unsigned long max_chinese = 0x9FFF;
// End 中文 unicode 區

const wchar_t *tokens = L"。？！\r\b\t\n?!";

typedef struct {
    wchar_t *url;
    wchar_t *title;
    wchar_t *context;
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
    wchar_t **sentances=NULL, **s_ptr_t=NULL;
    wchar_t *ptr=NULL, *buff=NULL;
    int cnt = 0;
    int cap = 32; // 初始容量, 三十二個寬字元
    sentances = (wchar_t**) malloc(sizeof(wchar_t*)*cap);
    if (sentances==NULL) exit(1);
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
                cap *= 2;
                s_ptr_t = NULL;
                s_ptr_t = (wchar_t**) malloc(sizeof(wchar_t*)*cap);
                if (s_ptr_t==NULL) exit(2);
                memcpy(s_ptr_t, sentances, sizeof(wchar_t*)*cnt);
                free(sentances);
                sentances = s_ptr_t;
                s_ptr_t = NULL;
            }
            sentances[cnt] = NULL; (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(ptr)+1));
            sentances[cnt] = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(ptr)+1));
            if (sentances[cnt]==NULL) exit(4);
            wcscpy(sentances[cnt], ptr);
            ++cnt;
        }
        ptr = wcstok(NULL, tokens, &buff);
    }
    s_ptr_t = NULL;
    s_ptr_t = (wchar_t**) malloc(sizeof(wchar_t*)*cnt);
    if (s_ptr_t==NULL) exit(3);
    memcpy(s_ptr_t, sentances, sizeof(wchar_t*)*cnt);
    free(sentances);
    sentances = s_ptr_t;
    s_ptr_t = NULL;
    *results = sentances;
    return cnt;
}

void format_line(wchar_t *str) {
    wchar_t *ptr = str;
    while(ptr!=NULL && *ptr!=0) {
        if (*ptr==L'\t' || *ptr==L'\b' || *ptr==L'\r') *ptr=L' ';
        if (*ptr==L'\n') { 
            *ptr=0;
            break;
        }
        ++ptr;
    }
}

int parse(void) {
    char filename[32];
    wchar_t *buffer = NULL, *ptr=NULL;
    wchar_t **sentances = NULL;
    FILE *fp = NULL;
    FILE *fout = NULL;
    fout = fopen("sentances.txt", "wb");
    news_record one_record;
    buffer = (wchar_t*)malloc(sizeof(wchar_t)*(buffer_limit+8));
    if (buffer==NULL) return -1;
    for (size_t fno=0; fno<file_numbers; ++fno) {
        get_file_path(filename, fno);
        fp = fopen(filename, "rb");
        if (fp==NULL) return -2;

        while(fgetws(buffer, buffer_limit, fp)!=NULL) {
            if(wcsncmp(buffer, L"@GAISRec:", 9)!=0) continue;
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
            s_cnt = tokenize(&sentances, one_record.context);
            free(one_record.context); one_record.context=NULL;
            for (size_t i=0; i<s_cnt; ++i) {
                fwprintf(fout, L"%ls\t%ls\t%ls\n", sentances[i], one_record.title, one_record.url);
                free(sentances[i]); sentances[i]=NULL;
            }
            free(sentances); sentances=NULL;
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

int main(void) {
    setlocale(LC_ALL, "");
    return parse();
}
