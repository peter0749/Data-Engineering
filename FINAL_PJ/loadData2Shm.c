#include <sys/types.h>
#include <sys/wait.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    struct shmid_ds shmd;
    struct ipc_perm perm;
    FILE *fp=NULL;
    FILE *fin=NULL;
    int shm_id=0, n_rows=0, n_cols=0;
    float *shm_ptr=NULL;
    unsigned int *int_data=NULL;
    fp=fopen("./ipc_addr", "w");
    if (fp==NULL) exit(1);
    fin=fopen("./.db/word_count/features.bin", "rb");
    if (fin==NULL) exit(2);
    fread(&n_rows, sizeof(n_rows), 1, fin);
    fread(&n_cols, sizeof(n_cols), 1, fin);
    int_data = (unsigned int*)malloc((long long)n_rows*n_cols*sizeof(unsigned int));
    shm_id = shmget(IPC_PRIVATE, (long long)n_rows*n_cols*sizeof(float), S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL);
    if (shm_id<0) {
        perror("Error on shmget()");
        exit(3);
    }
    
    shm_ptr = (float*)shmat(shm_id, NULL, 0);
    if (shm_ptr==(float*)-1) {
        perror("Error on shmat()");
        exit(4);
    }
    
    fread(int_data, sizeof(unsigned int), (long long)n_rows*n_cols, fin);
    fclose(fin);

    for (unsigned long long int i=0; i<(long long)n_rows*n_cols; ++i) 
        shm_ptr[i] = (float)int_data[i];
    free(int_data); int_data=NULL;

    if(shmdt(shm_ptr)==-1) {
        perror("Error on shmdt()");
        exit(5);
    }

    fprintf(fp, "%d %d %d\n", shm_id, n_rows, n_cols);
    fclose(fp);
    return 0; // process exit normally
}
