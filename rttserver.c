#include "p2mp.h"

#define BUF_SIZE 20

void usage()
{
  printf("rttserver port\n");
  printf("          port    - port to bind to\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int sock, ret, addr_size;
  char buf[BUF_SIZE];
  struct sockaddr_in src_addr, dest_addr;

  if(argc!=2) {
    usage();
  }

  src_addr.sin_family = AF_INET;
  src_addr.sin_addr.s_addr = INADDR_ANY;
  src_addr.sin_port = htons(atoi(argv[1]));

  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("rttserver : socket", errno);
  }

  if(bind(sock, (struct sockaddr*)&src_addr, sizeof(src_addr)) == -1) {
    close(sock);
    die("rttserver : bind failed!", errno);
  }

  while(1) {
    addr_size = sizeof(struct sockaddr_in);

    ret = recvfrom(sock,
                   buf,
                   BUF_SIZE-1,
                   0,
                   (struct sockaddr*)(&(dest_addr)),
                   &addr_size);
    if(ret == -1) {
      close(sock);
      die("rttserver : recvfrom failed!", errno);
    }
    buf[ret] = '\0';

    printf("received %s from %s:%d\n",
           buf,
           inet_ntoa(dest_addr.sin_addr),
           htons(dest_addr.sin_port));

    ret = sendto(sock,
                 "ack",
                 3,
                 0,
                 (struct sockaddr*)(&(dest_addr)),
                 sizeof(struct sockaddr_in));
    if(ret==-1) {
      close(sock);
      die("rttclient : sendto failed!", errno);
    }
  }

  return 0;
}
