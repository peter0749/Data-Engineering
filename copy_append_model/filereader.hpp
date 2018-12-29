#ifndef __INCLUDE_FILEREADER______
#define __INCLUDE_FILEREADER______
#include <cstdio>
#include <cstdlib>
#include <cstring>
int open_n_read_file(const char *filepath, unsigned char **ret_text) {
    FILE *fp=NULL;
    unsigned char *text=NULL;
    int file_size=0;
    int read_file_size=0;
    int text_size=0;
    fp = fopen(filepath, "rb");
    if (fp==NULL) exit(1);
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    text = new unsigned char[file_size/sizeof(unsigned char)+10];
    read_file_size = fread(text, sizeof(unsigned char), file_size, fp);
    text[read_file_size] = 0;
    fclose(fp); fp=NULL;
    if(ret_text!=NULL) *ret_text = text;
    else delete[] text;
    return read_file_size;
}
#endif
