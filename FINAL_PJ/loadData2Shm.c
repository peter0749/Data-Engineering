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
    void *shm_ptr=NULL;
    fp=fopen("./ipc_addr", "w");
    if (fp==NULL) exit(1);
    fin=fopen("./.db/word_count/features.bin", "rb");
    if (fin==NULL) exit(2);
    fread(&n_rows, sizeof(n_rows), 1, fin);
    fread(&n_cols, sizeof(n_cols), 1, fin);
    shm_id = shmget(IPC_PRIVATE, (long long)n_rows*n_cols*sizeof(unsigned int), S_IRUSR | S_IWUSR | IPC_CREAT | IPC_EXCL);
    if (shm_id<0) {
        perror("Error on shmget()");
        exit(3);
    }
    
    shm_ptr = shmat(shm_id, NULL, 0);
    if (shm_ptr==(char*)-1) {
        perror("Error on shmat()");
        exit(4);
    }
    
    fread(shm_ptr, sizeof(unsigned int), (long long)n_rows*n_cols, fin);
    fclose(fin);

    if(shmdt(shm_ptr)==-1) {
        perror("Error on shmdt()");
        exit(5);
    }

    fprintf(fp, "%d %d %d\n", shm_id, n_rows, n_cols);
    fclose(fp);
    return 0; // process exit normally
}
