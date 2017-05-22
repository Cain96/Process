#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#define BUFSIZE 256
#define TIMEOUT 10

void myalarm(int sec) {

  if ((pid=fork())== -1) {
    perror("fork failed.");
    exit(1);
  }
  if (pid == 0) { /* Child process */
    sleep(sec);
    if (kill(ppid,SIGALRM) == -1) {
      perror("kill failed.");
      exit(1);
    }
    exit(0);
  }
}

void timeout()
{
  printf("This program is timeout.\n");
  exit(0);
}

int main()
{
  char buf[BUFSIZE];

  if(signal(SIGALRM,timeout) == SIG_ERR) {
    perror("signal failed.");
    exit(1);
  }

  myalarm(TIMEOUT);

  while (fgets(buf, BUFSIZE, stdin) != NULL) {
    printf("echo: %s",buf);
    myalarm(TIMEOUT);
  }
}
