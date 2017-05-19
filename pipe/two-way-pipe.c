#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFSIZE 256
int main(int argc, char *argv[])
{
  char p_buf[BUFSIZE], c_buf[BUFSIZE];
  int fd1[2], fd2[2];
  int pid, msglen, status;

  if (argc != 3) {
    printf("bad argument.\n");
    exit(1);
  }
  if (pipe(fd1) == -1) {
    perror("pipe1 failed.");
    exit(1);
  }
  if (pipe(fd2) == -1) {
    perror("pipe2 failed.");
    exit(1);
  }
  if ((pid=fork())== -1) {
    perror("fork failed.");
    exit(1);
  }
  if (pid == 0) { /* Child process */

    close(fd1[0]);
    msglen = strlen(argv[1]) + 1;
    if (write(fd1[1], argv[1], msglen) == -1) {
      perror("pipe write.");
      exit(1);
    }

    close(fd2[1]);
    if (read(fd2[0], p_buf, BUFSIZE) == -1) {
      perror("pipe read.");
      exit(1);
    }

    printf("Message from parent process: \n");
    printf("\t%s\n",p_buf);
    exit(0);
  } else { /* Parent process */

    close(fd2[0]);
    msglen = strlen(argv[2]) + 1;
    if (write(fd2[1], argv[2], msglen) == -1) {
      perror("pipe write.");
      exit(1);
    }

    close(fd1[1]);
    if (read(fd1[0], c_buf, BUFSIZE) == -1) {
      perror("pipe read.");
      exit(1);
    }
    wait(&status);
    printf("Message from child process: \n");
    printf("\t%s\n",c_buf);
  }
}
