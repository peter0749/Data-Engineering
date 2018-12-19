#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include "record_structure.h"

int main(int argc, char **argv) {
    setlocale(LC_ALL, "zh_TW.UTF-8");
    setlocale(LC_CTYPE, "zh_TW.UTF-8");
    using namespace std;
    news_record_structure record;
    FILE *fp=NULL;
    char fpath[64];
    int id=0;
    if (argc!=2) exit(1);
    id = atoi(argv[1]);
    sprintf(fpath, "./.db/%lu.dat", id);
    read_news_record(&record, fpath);

    wcout << record.url << endl;
    wcout << record.title << endl;
    wcout << record.content << endl;

    free(record.content); record.content=NULL;
    
    return 0;
}
