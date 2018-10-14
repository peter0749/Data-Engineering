#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const char *parameter_patterns[5] = {"-d", "-k", "-c", "-r", "-n"};
const char *default_args[2] = {
    "\n", "" /* -d, -k */
};
const char *parameters[2]; /* -d, -k */
int set_parameters[3]={0}; /* -c, -r, -n */

int parse_parameter(const int argc, const char** args, const char* pattern) {
    int i=0;
    for (i=0; i<argc; ++i) {
        if(strstr(args[i], pattern)!=NULL) return i;
    }
    return -1;
}

void get_args(const int argc, const char** args, const char **parameters, int *set_parameters) {
    /** parse all parameters in following order
     * -d record_delimiter 
     * -k key_pat
     * -c case_insensitive
     * -r reverse order
     * -n numerical comparison
     */
    int i = 0, argspos=-1;
    for (i=0; i<2; ++i) {
        argspos = parse_parameter(argc, args, parameter_patterns[i]);
        parameters[i] = argspos<0?default_args[i]:args[argspos+1];
    }
    for (i=0; i<3; ++i) {
        set_parameters[i] = parse_parameter(argc, args, parameter_patterns[i+2])<0?0:1;
    }
}

int comp(const void *a, const void *b) {
    int e=0, f=0;
    const char *c = *(const char**)a;
    const char *d = *(const char**)b;
    int val = 0;
    if (parameters[1][0]!='\0') { /* has field */
        /* not robust enough. need to handle more exceptions */
        c = strstr(c, parameters[1]); /* jump to that field */
        d = strstr(d, parameters[1]); /* ,, */
    }
    if (set_parameters[2]) { /* numerical comparison? */
        while(c!=NULL&&*c!='\0'&&!isdigit(*c)) ++c;
        while(d!=NULL&&*d!='\0'&&!isdigit(*d)) ++d;
        e = atoi(c);
        f = atoi(d);
        val = e-f;
    } else { /* lexical order */
        val = set_parameters[0]?strcasecmp(c,d):strcmp(c,d); /* case insensitive? */
    }
    return set_parameters[1]?-val:val; /* reverse order? */
}

int reader(const char *filename, char ***results) {
    int i=0;
    char ch=0;
    FILE *fp=NULL;
    fp = fopen(filename, "r");
    if (fp==NULL) {
        fprintf(stderr, "Couldn't open file %s\n", filename);
        exit(2);
    }

    char *buffer=NULL, *new_buffer=NULL;
    char **rows=NULL, **new_rows=NULL;
    char *record_strings = NULL;
    int buffer_cap=1024;
    int buffer_cnt=0;
    int rows_cap=1024;
    int rows_cnt=0;
    int string_length=0;
    int delimiter_length = strlen(parameters[0]);
    buffer = (char*)malloc(sizeof(char )*buffer_cap);
    rows   = (char**)malloc(sizeof(char*)*rows_cap);
    if (buffer==NULL||rows==NULL) {
        fprintf(stderr, "Couldn't allocate more memory\n");
        exit(3);
    }

    for(;;) {
        ch = fgetc(fp);
        if (buffer_cnt+1==buffer_cap) {
            buffer_cap *= 2;
            new_buffer = NULL;
            new_buffer = (char*)malloc(sizeof(char )*buffer_cap);
            memcpy(new_buffer, buffer, sizeof(char)*buffer_cnt);
            free(buffer);
            buffer=new_buffer; new_buffer=NULL;
        }
        if (ch!=EOF) {
            buffer[buffer_cnt++] = ch;
            buffer[buffer_cnt  ] = '\0'; /* tail */
        }
        string_length = buffer_cnt - delimiter_length;
        if(string_length>0 && (ch==EOF || strncmp(buffer+string_length, parameters[0], delimiter_length)==0)) { /* new record */
            if (rows_cnt==rows_cap) {
                rows_cap *= 2;
                new_rows = NULL;
                new_rows = (char**)malloc(sizeof(char*)*rows_cap);
                memcpy(new_rows, rows, sizeof(char*)*rows_cnt);
                free(rows);
                rows=new_rows; new_rows=NULL;
            }
            record_strings = NULL;
            record_strings = (char*)malloc(sizeof(char)*(string_length+1));
            memcpy(record_strings, buffer, sizeof(char)*string_length);
            record_strings[string_length] = '\0';
            
            rows[rows_cnt++] = record_strings;
            buffer_cnt = 0; /* reset. read next record */
        }
        if (ch==EOF) break;
    }

    free(buffer); buffer=NULL;
    new_rows = (char**)malloc(sizeof(char**)*rows_cnt);
    memcpy(new_rows, rows, sizeof(char**)*rows_cnt);
    free(rows);
    rows = new_rows; new_rows=NULL;

    fclose(fp); fp=NULL;
    *results = rows;
    return rows_cnt;
}

int main(const int argc, const char **argv) {
    int i=0;
    const char *filename=NULL;
    char **records = NULL;
    int records_cnt = 0;
    if (argc<2) {
        fprintf(stderr, "Usage:\nrsort filename [-d delimeter | -k field | -n numeric comparison | -r reverse sort | -c case insensitive]\n");
        exit(2);
    }
    get_args(argc, argv, parameters, set_parameters);
    records_cnt = reader(argv[1], &records); /* need to free! */

    /* now sort */
    qsort((void*)records, records_cnt, sizeof(char*), comp); 

    for (i=0; i<records_cnt; ++i) {
        fputs(parameters[0], stdout);
        fputs(records[i], stdout);
    }

    free(records); records=NULL;
    return 0;
}
