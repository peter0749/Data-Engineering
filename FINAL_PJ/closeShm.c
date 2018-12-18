#include <sys/types.h>
#include <sys/wait.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    FILE *fp=NULL;
    int shm_id=0;
    fp=fopen("./ipc_addr", "r");
    fscanf(fp, "%d", &shm_id);
    fclose(fp);
    if (shmctl(shm_id, IPC_RMID, NULL)<0) {
        perror("Error on shmctl()");
        exit(1);
    }
    return 0; // process exit normally
}
