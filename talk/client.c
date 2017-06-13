#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define STRLEN 128
#define TIMEOUT 10

int sock;

void myalarm(int sec) {
  static int pid;

  struct sigaction sa;
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = SA_NOCLDWAIT;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    exit(1);
  }

  if(pid != 0){
    kill(pid, SIGTERM);
  }

  if ((pid=fork()) == -1) {
    perror("fork failed.");
    exit(1);
  }
  if (pid == 0) { /* Child process */
    sleep(sec);
    if (kill(getppid(),SIGALRM) == -1) {
      perror("kill failed.");
      exit(1);
    }
    exit(0);
  } else {
    return;
  }
}

void timeout()
{
  char msg[] = "This program is timeout.\nclosed\n";
  write(1, msg, sizeof(msg));
  close(sock);
  exit(0);
}

int main(int argc, char *argv[]) {
    char text[STRLEN];
    char *rtext;
    struct sockaddr_in svr;
    int reuse;
    struct hostent *host;
    int clen;
    char buf[1024];
    char rbuf[1024];
    int nbytes;
    fd_set rfds;
    struct timeval tv;


    /* 引数が1つであることの確認 */
    if (argc != 2) {
        perror("Illegal arguments");
        exit(1);
    }

    if(signal(SIGALRM,timeout) == SIG_ERR) {
      perror("signal failed.");
      exit(1);
    }

    /* ソケットの生成 */
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket");
        exit(1);
    }
    /* ソケットアドレス再利用の指定 */
    reuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
  		perror("setsockopt");
  		exit(1);
	  }

    memset((char *) &svr, 0, sizeof(svr));
    /* client_addr構造体にサーバの設定 */
    if((host = gethostbyname(argv[1])) == NULL){
  		perror("setIP");
  		exit(1);
    }
    bcopy(host->h_addr, &svr.sin_addr, host->h_length);
    svr.sin_family = AF_INET;
    svr.sin_port = htons(10130);

    /* connectでhostと接続 */
    if (connect(sock, (struct sockaddr *) &svr, sizeof(svr)) > 0) {
        perror("connect");
        close(sock);
        exit(1);
    }

    myalarm(TIMEOUT);

    while(1){
                /* 入力を監視するファイル記述子の集合を変数 rfds に
セットする */
  	    FD_ZERO(&rfds); /* rfds を空集合に初期化 */
        FD_SET(0,&rfds); /* 標準入力 */
        FD_SET(sock,&rfds); /* クライアントを受け付けたソケット */
        /* 監視する待ち時間を 1 秒に設定 */
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        /* 標準入力とソケットからの受信を同時に監視する */
        if(select(sock+1,&rfds,NULL,NULL,&tv)>0) {
            if(FD_ISSET(0,&rfds)) {
                /* 標準入力から入力があったなら */
                /* 標準入力から読み込みクライアント
に送信 */
                myalarm(TIMEOUT);
                memset(buf, '\0', 1024);
                if(read(0,buf, 1024)!=0){
                    write(sock, buf, sizeof(buf));
                }else{
                    break;
                }
            }
            if(FD_ISSET(sock,&rfds)) {
                /* ソケットから受信したなら */
                /* ソケットから読み込み端末に出力 */
                myalarm(TIMEOUT);
                if ((nbytes = read(sock, rbuf, sizeof(rbuf))) < 0) {
                    perror("read");
                } else if(nbytes == 0){
            			break;
            		} else {
                  write(1, rbuf, nbytes);
                }
            }
        }
    } /* 繰り返す */
    close(sock);
    printf("closed\n");
}
