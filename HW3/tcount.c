#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <unistd.h>
#include <assert.h>

unsigned long memory_limit = 100;
unsigned long key_size = 8;
unsigned long table_size = 30;
unsigned long key_table_size = 0;
unsigned long ext_key_table_base = 0xFFFFFFFF; // inf
unsigned long crr_key_n = 0;
unsigned long virtual_table_size = 100;
unsigned long initialized_virtual_address = 0;
unsigned long hash_mod = 1000691;
const unsigned long hash_base = 311;
const unsigned long hash_base2 = 337;
FILE *hash_external=NULL;
FILE *key_external=NULL;

const unsigned long prime_list[] = {
    2027, 5023, 10061, 20051, 50051,
    100069, 200033, 500083, 
    1000691, 2000731, 5000759, 10000139,  
    20000327, 50000389, 100000073,  
    200000081, 500000057, 1000000123,  
    2000000243, 4000003013uL // want more? than go long long -.-
};

typedef struct {
    unsigned long key_pos;
    unsigned long count;
    unsigned long next;
} hash_node;

hash_node *hash_table = NULL;
wchar_t *key_table = NULL;

void init_hashtable(void) {
    int i;
    size_t prime_n = sizeof(prime_list)/sizeof(prime_list[0]); 
    hash_mod = prime_list[prime_n-1];
    for (i=prime_n-1; i>=0; --i) {
        if (prime_list[i]<=virtual_table_size) {
            hash_mod = prime_list[i];
            break;
        }
    }
    hash_table = NULL;
    hash_table = (hash_node*)malloc(sizeof(hash_node)*table_size);
    if (hash_table==NULL) exit(1);
    key_table = NULL;
    key_table = (wchar_t*)malloc(sizeof(wchar_t)*key_table_size);
    if (key_table==NULL) exit(2);
    hash_external=NULL;
    hash_external = tmpfile();
    if (hash_external==NULL) exit(3);
    key_external = tmpfile();
    if (key_external==NULL) exit(4);
    memset(hash_table, 0x00, sizeof(hash_node)*table_size); // if next==0 -> terminal and hash_table[0] is dummy node
    initialized_virtual_address = table_size;
}

void destroy_hashtable(void) {
    if (hash_table!=NULL) { free(hash_table); hash_table=NULL; }
    if (key_table!=NULL) { free(key_table); key_table=NULL; }
    if (hash_external!=NULL) { fclose(hash_external); hash_external=NULL; }
    if (key_external!=NULL) { fclose(key_external); key_external=NULL; }
}

unsigned long get_hash_value(const wchar_t *str, unsigned long n) {
    unsigned long ans=0uL;
    while(n--) {
        ans = ((ans*hash_base)%hash_mod + (unsigned long)(*str)) % hash_mod;
        ++str;
    }
    return ans+1; // shift 1
}

void insert_key_table(wchar_t *str, unsigned long n) {
    const wchar_t zero=0;
    if (crr_key_n+n+1<=key_table_size) { // space is enough
        wcsncpy(&key_table[crr_key_n], str, n);
        crr_key_n += n;
        key_table[crr_key_n++]=0;
        return;
    }
    if (ext_key_table_base==0xFFFFFFFF) ext_key_table_base = crr_key_n;
    unsigned long offset = (crr_key_n-ext_key_table_base) * sizeof(wchar_t);
    fseek(key_external, offset, SEEK_SET);
    fwrite(str, sizeof(wchar_t), n, key_external);
    fwrite(&zero, sizeof(wchar_t), 1, key_external);
    crr_key_n += n+1;
}

wchar_t* read_key_table(unsigned long index, wchar_t **ext_str) {
    if (index<ext_key_table_base) return &key_table[index];
    // in disk
    unsigned offset = (index-ext_key_table_base) * sizeof(wchar_t);
    wchar_t *str = (wchar_t*)malloc(sizeof(wchar_t)*(key_size+1));
    fseek(key_external, offset, SEEK_SET);
    fread(str, sizeof(wchar_t), (key_size+1), key_external);
    *ext_str = str;
    return NULL;
}

void check_hash_table_boundary(unsigned long address) {
    unsigned long offset = 0, count = 0, i = 0;
    hash_node *zero = NULL;
    if (address>=initialized_virtual_address) {
        zero = (hash_node*)malloc(sizeof(hash_node));
        zero->key_pos = 0;
        zero->next = 0;
        zero->count = 0;
        offset = (initialized_virtual_address - table_size) * sizeof(hash_node);
        count  = address - initialized_virtual_address + 1;
        fseek(hash_external, offset, SEEK_SET);
        for (i=0; i<count; ++i)
            fwrite(zero, sizeof(hash_node), 1, hash_external);
        if (zero!=NULL) { free(zero); zero=NULL; }
        initialized_virtual_address += count;
    }
}

hash_node* read_hash_table(unsigned long address, hash_node **external) {
    if (address<table_size) {
        return &hash_table[address];
    }
    // in disk
    check_hash_table_boundary(address);
    unsigned long offset = (address-table_size) * sizeof(hash_node);
    hash_node *node = (hash_node*)malloc(sizeof(hash_node));
    fseek(hash_external, offset, SEEK_SET);
    fread(node, sizeof(hash_node), 1, hash_external);
    *external = node;
    return NULL;
}

void write_hash_table(unsigned long address, unsigned long key_pos, unsigned long count, unsigned long next) {
    if (address<table_size) {
        hash_table[address].key_pos = key_pos;
        hash_table[address].count = count;
        hash_table[address].next = next;
        return;
    }
    // write to disk
    check_hash_table_boundary(address);
    unsigned long offset = (address-table_size) * sizeof(hash_node);
    hash_node temp;
    temp.key_pos = key_pos;
    temp.count = count;
    temp.next = next;
    fseek(hash_external, offset, SEEK_SET);
    fwrite(&temp, sizeof(hash_node), 1, hash_external);
}

int match_from_address(unsigned long address, wchar_t *str, unsigned long n) {
    wchar_t *target=NULL, *external=NULL; 
    hash_node *node=NULL, *e_node=NULL;
    node = read_hash_table(address, &e_node);
    if (node==NULL) node = e_node;
    target = read_key_table(node->key_pos, &external);
    if (e_node!=NULL) { free(e_node); e_node=NULL; }
    node = NULL;
    int result = wcsncmp(target==NULL?external:target, str, n);
    if (external!=NULL) { free(external); external=NULL; }
    return result==0?1:0;
}

unsigned long search_hashtable(wchar_t *str, unsigned long n) {
    unsigned long address = get_hash_value(str, n);
    unsigned long next=0;
    hash_node *node=NULL, *external=NULL;
    int match=0;
    while (!(match=match_from_address(address, str, n))) {
        node=NULL; external=NULL;
        node = read_hash_table(address, &external);
        next = (node==NULL?external:node)->next;
        if (external!=NULL) { free(external); external=NULL; }
        if (next==0) break;
        address = next;
    }
    return match?address:0;
}

void dump_table(void) {
    unsigned long i=0;
    wchar_t *target=NULL, *external=NULL; 
    hash_node *node=NULL, *e_node=NULL;
    for (i=1; i<=hash_mod; ++i) {
        node=NULL; e_node=NULL;
        node = read_hash_table(i, &e_node);
        if (node==NULL) node = e_node;
        if (node->count!=0) {
            target=NULL; external=NULL;
            target = read_key_table(node->key_pos, &external);
            fwprintf(stdout, L"%u, %ls\n", node->count, target==NULL?external:target);
            if (external!=NULL) { free(external); external=NULL; }
        }
        if (e_node!=NULL) { free(e_node); e_node=NULL; }
    }
}

void insert_hashtable(wchar_t *str, unsigned long n) {
    unsigned long address = get_hash_value(str, n);
    int match=0;
    unsigned long next=0;
    hash_node *node=NULL, *external=NULL;
    while (!(match=match_from_address(address, str, n))) {
        node=NULL; external=NULL;
        node = read_hash_table(address, &external);
        next = (node==NULL?external:node)->next;
        if (external!=NULL) { free(external); external=NULL; }
        if (next==0) break;
        address = next;
    }
    if (match) { // update
        node=NULL; external=NULL;
        node = read_hash_table(address, &external);
        if (node==NULL) node = external;
        ++(node->count);
        write_hash_table(address, node->key_pos, node->count, node->next);
        if (external!=NULL) { free(external); external=NULL; }
        node = NULL;
        return;
    }
    unsigned long new_address = address;
    unsigned long fails=0;
    for(;;) {
        node=NULL; external=NULL;
        node = read_hash_table(new_address, &external);
        unsigned long count = (node==NULL?external:node)->count;
        if (external!=NULL) { free(external); external=NULL; }
        if (count==0) break;
        ++fails;
        assert(fails<hash_mod); // hash table is not full
        new_address = ((new_address-1)+hash_base2)%hash_mod+1; // note that the address is shifted by 1
    }
    if (fails>0) {
        node=NULL; external=NULL;
        node = read_hash_table(address, &external);
        if (node==NULL) node = external;
        write_hash_table(address, node->key_pos, node->count, new_address);
        if (external!=NULL) { free(external); external=NULL; }
        node = NULL;
    }
    write_hash_table(new_address, crr_key_n, 1, 0); // address, key_pos, count, next
    insert_key_table(str, n);
}

int parse_parameter(const int argc, const char** args, const char* pattern) {
    int i=0;
    for (i=0; i<argc; ++i) {
        if(strstr(args[i], pattern)!=NULL) return i;
    }
    return -1;
}

void get_args(const int argc, const char** args) {
    /** parse all parameters in following order
     * -m memory size (in MB)
     * -s key size (in byte)
     * -h hash table size (in MB)
     * -v hash table size (virtual)
     */
    int i = 0, argspos=-1;

    argspos = parse_parameter(argc, args, "-m");
    memory_limit = argspos<0?memory_limit:atol(args[argspos+1]);

    argspos = parse_parameter(argc, args, "-s");
    key_size = argspos<0?key_size:atol(args[argspos+1]);

    argspos = parse_parameter(argc, args, "-h");
    table_size = argspos<0?table_size:atol(args[argspos+1]);

    argspos = parse_parameter(argc, args, "-v");
    virtual_table_size = argspos<0?virtual_table_size:atol(args[argspos+1]);

    if (virtual_table_size<table_size) virtual_table_size = table_size;

    key_table_size = memory_limit - table_size;

    table_size <<= 20uL; // x1024x1024
    table_size = (table_size-sizeof(hash_node)+1) / sizeof(hash_node);
    key_table_size <<= 20uL;
    key_table_size = (key_table_size-sizeof(wchar_t)+1) / sizeof(wchar_t);
    virtual_table_size <<= 20uL;
    virtual_table_size = (virtual_table_size-sizeof(hash_node)+1) / sizeof(hash_node);
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

void counting(FILE *fp) {
    wchar_t *row = NULL;
    wchar_t *read_row = NULL;
    unsigned long read_buffer_size = 16384;
    row = (wchar_t*)malloc(sizeof(wchar_t) * (key_size+5));
    if (row==NULL) exit(1);
    read_row = (wchar_t*)malloc(sizeof(wchar_t) * (read_buffer_size+5));
    if (read_row==NULL) exit(2);
    while(fgetws(read_row, read_buffer_size, fp)!=NULL) {
        wcsncpy(row, read_row, key_size);
        row[key_size] = 0;
        format_line(row);
        unsigned long len = wcslen(row);
        row[len] = 0;
        insert_hashtable(row, len);
    }
    free(row); row=NULL;
    free(read_row); read_row=NULL;
}

int main(const int argc, const char **argv) {
    FILE *fp=NULL;
    setlocale(LC_ALL, ""); // 使用這個， fgetws 才不會出錯
    if (argc==2 && strncmp("--help", argv[1], 6)==0) {
        fprintf(stderr, "Usage:\ntsort filename [-m memory_limit (in MB) | -s key_size (in byte) | -h hash_table_size (in MB) | -v virtual_hash_table_size (in MB) ]\n");
        return 0;
    }
    get_args(argc, argv);
    if (argc<2 || !check_exist(argv[1]) ) fp=stdin;
    else fp = fopen(argv[1], "rb");
    /* Process data */

    init_hashtable();
    counting(fp);
    dump_table();
    destroy_hashtable();

    /* End process data */
    if (fp!=NULL) {
        fclose(fp); fp=NULL;
    }
    return 0;
}
