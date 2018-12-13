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
    setlocale(LC_ALL, "");
    using namespace std;
    news_record_structure record;
    FILE *fp=NULL;
    char fpath[64];
    int id=0;
    if (argc!=2) exit(1);
    id = atoi(argv[1]);
    sprintf(fpath, "./.db/%lu.dat", id);
    read_news_record(&record, fpath);

    wcout << L"URL: " << record.url << endl;
    wcout << L"TITLE: " << record.title << endl;
    wcout << L"CONTEXT: " << record.content << endl;
    
    return 0;
}
