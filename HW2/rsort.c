#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <assert.h>
#include "msort.h"

typedef struct {
    size_t n_chunk;  /* Total number of chunks. */
    char has_head;   /* Data has head */
    FILE **temp_fp;  /* split data file pointer. (# = n_chunk) */
} split_sort_handler;

typedef struct {
    size_t n_record;
    char **data;
    char has_head;
} record_struct;

/*mymergesort((void*)rows, rows_cnt, sizeof(char*), cpu_count, comp);  */
typedef struct {
    void *data;
    size_t cnt;
    size_t size;
    int cpu_cnt;
    int (*cmp)(const void *, const void *);
} PARALLEL_SORT_ARGS;

void *msort_wrapper(void *init) {
    PARALLEL_SORT_ARGS *args = (PARALLEL_SORT_ARGS*)init;
    if (args->cpu_cnt>1)
        mymergesort(args->data, args->cnt, args->size, args->cpu_cnt, args->cmp);  
    else
        qsort(args->data, args->cnt, args->size, args->cmp); 
    pthread_exit(0);
    return NULL;
}

const char *parameter_patterns[9] = {"-d", "-k", "-m", "-f", "-j", "-c", "-r", "-n", "-s"};
const char *default_args[5] = {
    "\n", NULL, "100", NULL, "1" /* -d, -k, -m, -f, -j */
};
const char *parameters[5]; /* -d, -k, -m, -f, -j */
int set_parameters[4]={0}; /* -c, -r, -n, -s */
unsigned int cpu_count = 1;

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
    for (i=0; i<5; ++i) {
        argspos = parse_parameter(argc, args, parameter_patterns[i]);
        parameters[i] = argspos<0?default_args[i]:args[argspos+1];
    }
    for (i=0; i<4; ++i) {
        set_parameters[i] = parse_parameter(argc, args, parameter_patterns[i+5])<0?0:1;
    }
    cpu_count = atoi(parameters[4]);
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
            if (c==NULL) return -1;
            d = strstr(d, parameters[1]); /* ,, */
            if (d==NULL) return  1;
        }
        if (set_parameters[2]) { /* numerical comparison? */
            val = 0;
            while(c!=NULL&&*c!='\0'&&!isdigit(*c)) ++c, ++val;
            if (val>0 && *(c-1)=='-') --c;
            val = 0;
            while(d!=NULL&&*d!='\0'&&!isdigit(*d)) ++d, ++val;
            if (val>0 && *(d-1)=='-') --d;
            e = atoi(c);
            f = atoi(d);
            val = e-f;
        } else { /* lexical order */
            val = set_parameters[0]?strcasecmp(c,d):strcmp(c,d); /* case insensitive? */
        }
    }
    return set_parameters[1]?-val:val; /* reverse order? */
}

int comp_record(const record_struct *a, const record_struct *b) {
    /* precidition: a or b as data */
    if (a->n_record==0) return  1;
    if (b->n_record==0) return -1;
    return comp(a->data, b->data);
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
#define GROW_ALIGN(X, X_new, cap, cnt, type)  { cap *= 2; \
    X_new = NULL; \
    X_new = (type*)aligned_alloc(sizeof(type), sizeof(type)*cap); \
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
    pthread_t *sort_thread=NULL;
    PARALLEL_SORT_ARGS *msort_args=NULL;
    unsigned long tmp_fp_cap=4;
    unsigned long tmp_fp_cnt=0;
    unsigned long buffer_cap=1024;
    unsigned long buffer_cnt=0;
    unsigned long rows_cap=1024;
    unsigned long rows_cnt=0;
    unsigned long long mem_use=0;
    unsigned long chunk_n=0;
    int string_length=0;
    unsigned long delimiter_length = strlen(parameters[0]);
    char has_head = 2;
    const unsigned long long buffer_limit = atoll(parameters[2]) * (1uLL<<20uLL); /* KB=2^10, MB=2^20 */
    buffer = (char*)aligned_alloc(sizeof(char), sizeof(char )*buffer_cap);
    rows   = (char**)aligned_alloc(sizeof(char*), sizeof(char*)*rows_cap);
    tmp_fp = (FILE**)malloc(sizeof(FILE*)*tmp_fp_cap);
    if (buffer==NULL||rows==NULL||tmp_fp==NULL) {
        fprintf(stderr, "Couldn't allocate more memory\n");
        exit(3);
    }

    for(;;) {
        ch = (max_rec>0 && rows_cnt>=max_rec)?EOF:fgetc(fp);
        if (buffer_cnt+1==buffer_cap) {
            GROW_ALIGN(buffer, new_buffer, buffer_cap, buffer_cnt, char);
        }
        if (ch!=EOF) {
            buffer[buffer_cnt++] = ch;
            buffer[buffer_cnt  ] = '\0'; /* tail */
        }
        string_length = buffer_cnt - delimiter_length;
        if(string_length>0 && (ch==EOF || strncmp(buffer+string_length, parameters[0], delimiter_length)==0)) { /* new record */
            if (rows_cnt==rows_cap) {
                GROW_ALIGN(rows, new_rows, rows_cap, rows_cnt, char* );
            }
            record_strings = NULL;
            if (has_head==2 && (has_head = string_length > delimiter_length && strncmp(buffer, parameters[0], delimiter_length)==0)) {
                string_length -= delimiter_length;
                record_strings = (char*)aligned_alloc(sizeof(char), sizeof(char)*(string_length+1));
                memcpy(record_strings, buffer+delimiter_length, sizeof(char)*string_length);
                record_strings[string_length] = '\0';
            } else {
                record_strings = (char*)aligned_alloc(sizeof(char), sizeof(char)*(string_length+1));
                memcpy(record_strings, buffer, sizeof(char)*string_length);
                record_strings[string_length] = '\0';
            }
            rows[rows_cnt++] = record_strings;
            buffer_cnt = 0; /* reset. read next record */
            mem_use += (unsigned long long)(sizeof(char)*(string_length+1));
        }
        if (results!=NULL && (mem_use>=buffer_limit || ch==EOF)) { /* write file */

            /* wait previous sort thread to terminate */
            if (sort_thread!=NULL) {
                pthread_join(*sort_thread, NULL);
                free(sort_thread);
                sort_thread=NULL;
                FILE *temp = tmpfile(); /* w+ mode */
                writeout(temp, (char**)msort_args->data, msort_args->cnt, 0); /* write first delimeter */
                fputs("\n", temp);
                fseek(temp, 0, SEEK_SET); /* move to begining of the file */
                /* remember filepaths */
                if (tmp_fp_cnt==tmp_fp_cap) {
                    GROW(tmp_fp, new_tmp_fp, tmp_fp_cap, tmp_fp_cnt, FILE* );
                }
                tmp_fp[tmp_fp_cnt++] = temp;
                char **del_ptr = (char**)msort_args->data;
                for (i=0; i<msort_args->cnt; ++i) {
                    free(del_ptr[i]); del_ptr[i]=NULL;
                }
                free(del_ptr); del_ptr=NULL;
                free(msort_args);
                msort_args=NULL;
            }
            
            sort_thread = (pthread_t*)malloc(sizeof(pthread_t));
            assert(sort_thread!=NULL);

            msort_args = NULL;
            msort_args = (PARALLEL_SORT_ARGS*)malloc(sizeof(PARALLEL_SORT_ARGS));
            assert(msort_args!=NULL);

            msort_args->data=(void*)rows;
            msort_args->cnt =rows_cnt;
            msort_args->size=sizeof(char*);
            msort_args->cpu_cnt=cpu_count;
            msort_args->cmp=comp;
            assert(pthread_create(sort_thread, NULL, msort_wrapper, (void*)msort_args)==0); /*now sort*/
            /* sort in backgraound. continue to read file */
            
            rows = (char**)aligned_alloc(sizeof(char*), sizeof(char*)*rows_cap);
            rows_cnt = 0;
            mem_use = 0;
        }
        if (ch==EOF) break; 
    }
    free(buffer); buffer=NULL;
    if (sort_thread!=NULL) { /* complete final part */
        pthread_join(*sort_thread, NULL);
        free(sort_thread);
        sort_thread=NULL;
        FILE *temp = tmpfile(); /* w+ mode */
        writeout(temp, (char**)msort_args->data, msort_args->cnt, 0); /* write first delimeter */
        fputs("\n", temp);
        fseek(temp, 0, SEEK_SET); /* move to begining of the file */
        /* remember filepaths */
        if (tmp_fp_cnt==tmp_fp_cap) {
            GROW(tmp_fp, new_tmp_fp, tmp_fp_cap, tmp_fp_cnt, FILE* );
        }
        tmp_fp[tmp_fp_cnt++] = temp;
        char **del_ptr = (char**)msort_args->data;
        for (i=0; i<msort_args->cnt; ++i) {
            free(del_ptr[i]); del_ptr[i]=NULL;
        }
        free(del_ptr); del_ptr=NULL;
        free(msort_args);
        msort_args=NULL;
    }

    if (results!=NULL) {
        for (i=0; i<rows_cnt; ++i) {
            free(rows[i]);
            rows[i]=NULL;
        }
        free(rows); rows=NULL;
        results->n_chunk  = tmp_fp_cnt;
        results->has_head = has_head;
        results->temp_fp  = tmp_fp;
    } else {
        records->n_record = rows_cnt;
        records->data = rows;
        records->has_head = has_head;
    }
#undef GROW
}

void merge_and_out(split_sort_handler *handler) {
#define LCH(X) ((X<<1)+1)
#define RCH(X) ((X<<1)+2)
#define PAR(X) ((X-1)>>1)
    /* precondition: every file pointer points to the beginning of each file */
    char complete_bt = (handler->n_chunk&1);  /* is complete binary tree */
    unsigned long node_num = 2 * (handler->n_chunk + (complete_bt==0?1:0)) - 1;   /* # of nodes (if not complete. add a dummy node) */
    unsigned long i=0;
    unsigned long *nodes = NULL;
    unsigned long K = handler->n_chunk;
    unsigned long hidden_nodes=0;
    FILE *out_fp = stdout;
    if (parameters[3]!=NULL) {
        out_fp = fopen(parameters[3], "wb");
        if (out_fp==NULL) exit(5);
    }
    record_struct *records = NULL;
    nodes = (unsigned long*)aligned_alloc(sizeof(unsigned long ), sizeof(unsigned long)*node_num);
    if (nodes==NULL) exit(3);
    memset(nodes, 0xFF, sizeof(unsigned long)*node_num); /* set infinity (kinda) */

    records = (record_struct*)aligned_alloc(sizeof(record_struct)+sizeof(record_struct)%4, sizeof(record_struct)*K);
    if (records==NULL) exit(4);
    
    for (i=0; i<K; ++i) split_sort(handler->temp_fp[i], NULL, records+i, 1); /* read first K first elements */

    if (!complete_bt) { /* if not strictly complete */
        --node_num; /* hide dummy node */
    }

    hidden_nodes = node_num - K;
#define NI2KI(X) (X-hidden_nodes)
#define KI2NI(X) (X+hidden_nodes)
    for (i=hidden_nodes; i<node_num; ++i) nodes[i] = NI2KI(i); /* initalize leaf nodes */

    /* initialize root and hidden node */
    for (i=node_num-1; i>0; --i) { /* zero based */
        unsigned long p  = nodes[PAR(i)];
        unsigned long ch = nodes[i];
        if( p >= K || comp_record( records+p, records+ch )>0 ) { /* if parent is not initialized or is bigger */
            nodes[PAR(i)] = nodes[i]; /* update */
        }
    }

    char first=1;
    for(;;) { /* while top element is non-empty */
        if (records[nodes[0]].n_record==0) break; 
        writeout(out_fp, records[nodes[0]].data, 1, first?handler->has_head:1);
        cleanup_record_struct(records+nodes[0]); /* pop current data */
        first=0;
        split_sort(handler->temp_fp[ nodes[0] ], NULL, records+nodes[0], 1); /* read new data from disk */
        unsigned long ni = KI2NI( nodes[0] );
        while (ni!=0) { /* until reach root */
            unsigned long pa = PAR(ni);
            unsigned long lch = LCH(pa);
            unsigned long rch = RCH(pa);
            ni = lch; /* assume that lch is smaller */
            if ( rch<node_num && comp_record( records+nodes[lch], records+nodes[rch] ) > 0 ) { /* right child exists and smaller than left child */
                ni = rch; /* change to rch */
            }
            nodes[pa] = nodes[ni]; /* update */
            ni = pa; /* bottom up */
        }
    }

    free(nodes); nodes=NULL;
    for (i=0; i<K; ++i) cleanup_record_struct(records+i);
    free(records);
    records=NULL;
    fputs("\n", out_fp);
    if (out_fp!=NULL && out_fp!=stdout) fclose(out_fp);
    out_fp=NULL;
#undef KI2NI
#undef NI2KI
#undef LCH
#undef RCH
#undef PAR
}

int check_exist(const char *p) {
    FILE *f=NULL;
    f = fopen(p, "rb");
    if (f==NULL) return 0;
    else {
        fclose(f);
        return 1;
    }
}

int main(const int argc, const char **argv) {
    int i=0;
    FILE *fp=NULL;
    int records_cnt = 0;
    char has_head = 1;
    split_sort_handler handle;
    record_struct records;
    if (argc==2 && strncmp("--help", argv[1], 6)) {
        fprintf(stderr, "Usage:\nrsort filename [-d delimeter | -k field | -m memory_limit | -f output_filename (o.w. stdout) | -j n_jobs (number of cpu?) | -n (set numeric comparison) | -r (set reverse sort) | -c  (set case insensitive) | -s (set size_sort) ]\n");
        return 0;
    }
    get_args(argc, argv, parameters, set_parameters);
    if (argc<2 || !check_exist(argv[1]) ) fp=stdin;
    else fp = fopen(argv[1], "rb");
    split_sort(fp, &handle, &records, -1);
    fclose(fp); fp=NULL;
    merge_and_out(&handle);
    cleanup_split_sort_handle(&handle);
    return 0;
}
