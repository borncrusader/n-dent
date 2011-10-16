#include <stdio.h>

#include "p2mp.h"

void* receiver(void *args) {
  struct sockaddr_in ser;
  char buf[MSS];
  socklen_t len;
  unsigned char looper = 1;
  p2mp_pcb *pcb = (p2mp_pcb *)args;

  printf("receiver: waiting for acks..\n");

  while(looper) {
    recvfrom(pcb->sockfd, buf, MSS-1, 0, (struct sockaddr*)&ser, &len);
  }

  return;
}
