#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    size_t n_record; /* Total number of records. */
    size_t n_chunk;  /* Total number of chunks. */
    char has_head;   /* Data has head */
    FILE **temp_fp;  /* split data file pointer. (# = n_chunk) */
} split_sort_handler;

typedef struct {
    size_t n_record;
    char **data;
    char has_head;
} record_struct;

const char *parameter_patterns[8] = {"-d", "-k", "-m", "-f", "-c", "-r", "-n", "-s"};
const char *default_args[4] = {
    "\n", NULL, "100", NULL /* -d, -k, -m, -f */
};
const char *parameters[4]; /* -d, -k, -m, -f */
int set_parameters[4]={0}; /* -c, -r, -n, -s */

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
     * -m memory_limit (in MB)
     * -f output file path
     * -c case_insensitive
     * -r reverse order
     * -n numerical comparison
     * -s size_sort
     */
    int i = 0, argspos=-1;
    for (i=0; i<4; ++i) {
        argspos = parse_parameter(argc, args, parameter_patterns[i]);
        parameters[i] = argspos<0?default_args[i]:args[argspos+1];
    }
    for (i=0; i<4; ++i) {
        set_parameters[i] = parse_parameter(argc, args, parameter_patterns[i+4])<0?0:1;
    }
}

int comp(const void *a, const void *b) {
    int e=0, f=0;
    const char *c = *(const char**)a;
    const char *d = *(const char**)b;
    int val = 0;
    if ( set_parameters[3] ) { /* sort by "size" */
        val = strlen(c) - strlen(d);
    } else {
        if (parameters[1]!=NULL) { /* has field */
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
    }
    return set_parameters[1]?-val:val; /* reverse order? */
}

void writeout(FILE *fp, char **row, const unsigned long records_cnt, const char write_head) {
    unsigned long i=0;
    if (fp==NULL) fp=stdout;
    for (i=0; i<records_cnt; ++i) {
        if (i>0 || write_head) {
            fputs(parameters[0], fp);
        }
        fputs(row[i], fp);
    }
    fputs("\n", fp);
    if (fp!=stdout && fp!=NULL) fclose(fp);
    fp = NULL;
}

void cleanup_split_sort_handle(split_sort_handler *h) {
    size_t i=0;
    if (h==NULL) return;
    for (i=0; i<h->n_chunk; ++i) {
        fclose(h->temp_fp[i]); h->temp_fp[i]=NULL;
    }
    free(h->temp_fp); h->temp_fp=NULL;
}

void cleanup_record_struct(record_struct *r) {
    size_t i=0;
    if (r==NULL) return;
    for (i=0; i<r->n_record; ++i) {
        free(r->data[i]); r->data[i]=NULL;
    }
    free(r->data);
}

void split_sort(FILE *fp, split_sort_handler *results, record_struct *records, int max_rec) {
#define GROW(X, X_new, cap, cnt, type)  { cap *= 2; \
    X_new = NULL; \
    X_new = (type*)malloc(sizeof(type)*cap); \
    memcpy(X_new, X, sizeof(type)*cnt); \
    free(X); \
    X=X_new; X_new=NULL; }
    unsigned long i=0;
    char ch=0;
    FILE *temp=NULL;
    if (results==NULL && records==NULL) return;

    char *buffer=NULL, *new_buffer=NULL;
    char **rows=NULL, **new_rows=NULL;
    FILE **tmp_fp=NULL, **new_tmp_fp=NULL;
    char *record_strings = NULL;
    unsigned long tmp_fp_cap=4;
    unsigned long tmp_fp_cnt=0;
    unsigned long buffer_cap=1024;
    unsigned long buffer_cnt=0;
    unsigned long rows_cap=1024;
    unsigned long rows_cnt=0;
    unsigned long mem_use=0;
    unsigned long chunk_n=0;
    int string_length=0;
    unsigned long delimiter_length = strlen(parameters[0]);
    char has_head = 2;
    const unsigned long buffer_limit = atol(parameters[2]) * (1uL<<20); /* KB=2^10, MB=2^20 */
    buffer = (char*)malloc(sizeof(char )*buffer_cap);
    rows   = (char**)malloc(sizeof(char*)*rows_cap);
    tmp_fp = (FILE**)malloc(sizeof(FILE*)*tmp_fp_cap);
    if (buffer==NULL||rows==NULL||tmp_fp==NULL) {
        fprintf(stderr, "Couldn't allocate more memory\n");
        exit(3);
    }

    for(;;) {
        ch = (max_rec>0 && rows_cnt>=max_rec)?EOF:fgetc(fp);
        if (buffer_cnt+1==buffer_cap) {
            GROW(buffer, new_buffer, buffer_cap, buffer_cnt, char);
        }
        if (ch!=EOF) {
            buffer[buffer_cnt++] = ch;
            buffer[buffer_cnt  ] = '\0'; /* tail */
        }
        string_length = buffer_cnt - delimiter_length;
        if(string_length>0 && (ch==EOF || strncmp(buffer+string_length, parameters[0], delimiter_length)==0)) { /* new record */
            if (rows_cnt==rows_cap) {
                GROW(rows, new_rows, rows_cap, rows_cnt, char* );
            }
            record_strings = NULL;
            if (has_head==2 && (has_head = string_length > delimiter_length && strncmp(buffer, parameters[0], delimiter_length)==0)) {
                string_length -= delimiter_length;
                record_strings = (char*)malloc(sizeof(char)*(string_length+1));
                memcpy(record_strings, buffer+delimiter_length, sizeof(char)*string_length);
                record_strings[string_length] = '\0';
            } else {
                record_strings = (char*)malloc(sizeof(char)*(string_length+1));
                memcpy(record_strings, buffer, sizeof(char)*string_length);
                record_strings[string_length] = '\0';
            }
            rows[rows_cnt++] = record_strings;
            buffer_cnt = 0; /* reset. read next record */
            mem_use += sizeof(char)*(string_length+1);
        }
        if (results!=NULL && (mem_use>=buffer_limit || ch==EOF)) { /* write file */
            qsort((void*)rows, rows_cnt, sizeof(char*), comp);  /* now sort this portion */
            FILE *temp = tmpfile(); /* w+ mode */
            writeout(temp, rows, rows_cnt, 0); /* not write first delimeter to save memory */
            fseek(temp, 0, SEEK_SET); /* move to begining of the file */
            /* remember filepaths */
            if (tmp_fp_cnt==tmp_fp_cap) {
                GROW(tmp_fp, new_tmp_fp, tmp_fp_cap, tmp_fp_cnt, FILE* );
            }
            tmp_fp[tmp_fp_cnt++] = temp;
            for (i=0; i<rows_cnt; ++i) {
                free(rows[i]);
                rows[i]=NULL;
            }
            rows_cnt = 0;
            mem_use = 0;
        }
        if (ch==EOF) break; 
    }
    free(buffer); buffer=NULL;

    if (results!=NULL) {
        for (i=0; i<rows_cnt; ++i) {
            free(rows[i]);
            rows[i]=NULL;
        }
        free(rows); rows=NULL;
        results->n_record = rows_cnt;
        results->n_chunk  = tmp_fp_cnt;
        results->has_head = has_head;
        results->temp_fp  = tmp_fp;
    } else {
        records->n_record = rows_cnt;
        records->data = rows;
        records->has_head = has_head;
    }
}

int main(const int argc, const char **argv) {
    int i=0;
    FILE *fp=NULL;
    int records_cnt = 0;
    char has_head = 1;
    split_sort_handler handle;
    record_struct records;
    if (argc<2) {
        fprintf(stderr, "Usage:\nrsort filename [-d delimeter | -k field | -m memory_limit | -f output_filename (o.w. stdout) | -n (set numeric comparison) | -r (set reverse sort) | -c  (set case insensitive) | -s (set size_sort) ]\n");
        exit(2);
    }
    get_args(argc, argv, parameters, set_parameters);
    fp = fopen(argv[1], "r");
    split_sort(fp, NULL, &records, 3);
    writeout(stdout, records.data, records.n_record, records.has_head);
    cleanup_record_struct(&records);
    split_sort(fp, NULL, &records, 3);
    writeout(stdout, records.data, records.n_record, records.has_head);
    cleanup_record_struct(&records);
    fclose(fp); fp=NULL;

    fp = fopen(argv[1], "r");
    split_sort(fp, NULL, &records, 6);
    writeout(stdout, records.data, records.n_record, records.has_head);
    cleanup_record_struct(&records);
    fclose(fp); fp=NULL;
    return 0;
}
