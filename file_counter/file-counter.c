#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#define NUMPROCS 4
#define LOCK -1
#define UNLOCK 1
#define WAIT 0

void lock(int sid){
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = LOCK;
  sb.sem_flg = 0;
  if (semop(sid, &sb, 1) == -1) {
    perror("sem_wait semop error.");
    exit(1);
  }
}

void unlock(int sid){
  struct sembuf sb;
  sb.sem_num = 0;
  sb.sem_op = UNLOCK;
  sb.sem_flg = 0;
  if (semop(sid, &sb, 1) == -1) {
    perror("sem_signal semop error.");
    exit(1);
  }
}

int count1(FILE *ct, int sid, int pid)
{
  int count;


  if ((ct=fopen("counter", "r"))==NULL){
    exit(1);
  }
  fscanf(ct, "%d\n", &count);
  count++;
  fclose(ct);

  if ((ct=fopen("counter", "w"))==NULL){
    exit(1);
  }
  fprintf(ct, "%d\n", count);
  fclose(ct);

  unlock(sid);
  return count;
}

int main(){
  int i, count, pid, sid, status;
  FILE *ct;
  key_t key;
  setbuf(stdout, NULL); /* set stdout to be unbufferd */
  count = 0;

  if ((ct=fopen("counter", "w"))==NULL){
    exit(1);
  }
  fprintf(ct, "%d\n", count);
  fclose(ct);
  if ((key = ftok(".", 1)) == -1){
    fprintf(stderr,"ftok path does not exist.\n");
    exit(1);
  }
  if ((sid=semget(key, 1, 0666 | IPC_CREAT)) == -1) {
    perror("semget error.");
    exit(1);
  }
  if((semctl(sid,0,SETVAL,1))==EOF){
		printf("%s\n","ERR");
		exit(0);
	}
  for (i=0; i<NUMPROCS; i++) {
    lock(sid);
    if ((pid=fork())== -1) {
      perror("fork failed.");
      exit(1);
    }
    if (pid == 0) { /* Child process */
      count = count1(ct, sid, pid);
      printf("count = %d\n", count);
      exit(0);
    }
  }
  for (i=0; i<NUMPROCS; i++) {
    wait(&status);
  }
}
