#include "p2mp.h"
#include <time.h>

#define BUF_SIZE 20

void usage()
{
  printf("rttclient ip-addr port n\n");
  printf("          ip-addr - ip-address of the server\n");
  printf("          port    - port of the server\n");
  printf("          n       - number of packets to be sent\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int n, sock, i, addr_size, buf_size, ret;
  unsigned long rtt_sum = 0, rtt;
  char buf[BUF_SIZE];
  struct sockaddr_in dest_addr, src_addr;
  struct timeval start_time, end_time;

  if(argc!=4) {
    usage();
  }

  dest_addr.sin_family = AF_INET;
  dest_addr.sin_addr.s_addr = inet_addr(argv[1]);
  dest_addr.sin_port = htons(atoi(argv[2]));

  n = atoi(argv[3]);

  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("rttclient : socket", errno);
  }

  for(i=0; i<n; i++) {
    addr_size = sizeof(struct sockaddr_in);
    buf_size = snprintf(buf, BUF_SIZE, "pkt:%d", i);

    //start time
    gettimeofday(&start_time, NULL);

    ret = sendto(sock,
                 buf,
                 buf_size,
                 0,
                 (struct sockaddr*)(&(dest_addr)),
                 sizeof(struct sockaddr_in));
    if(ret==-1) {
      close(sock);
      die("rttclient : sendto failed!", errno);
    }

    ret = recvfrom(sock,
                   buf,
                   BUF_SIZE-1,
                   0,
                   (struct sockaddr*)(&(src_addr)),
                   &addr_size);
    if(ret==-1) {
      close(sock);
      die("rttclient : sendto failed!", errno);
    }
    buf[ret] = '\0';

    //end time
    gettimeofday(&end_time, NULL);

    rtt = (end_time.tv_sec*1000000)+end_time.tv_usec-
      ((start_time.tv_sec*1000000)+start_time.tv_usec);

    rtt_sum += rtt;

    printf("received %s from %s:%d, rtt = %ld us\n",
           buf,
           inet_ntoa(src_addr.sin_addr),
           htons(src_addr.sin_port),
           rtt);
  }

  printf("average rtt = %lf us\n", (double)rtt_sum/n);

  return 0;
}
