#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "p2mp.h"
#include "p2mpclient.h"

void* sender(void *args) {
  unsigned char looper = 1;
  struct timespec s_timer;
  p2mp_pcb *pcb = (p2mp_pcb*)args;

  pcb->sockfd = socket(AF_INET, SOCK_STREAM, 0);

  while(looper) {
    pthread_mutex_lock(&(pcb->win.win_lck));

    clock_gettime(CLOCK_REALTIME, &s_timer);
    s_timer.tv_sec += 2;

    pthread_cond_timedwait(&(pcb->win.win_cnd), &(pcb->win.win_lck), &s_timer);

    printf("sender: timedout (or) got data from rdt_send..\n");

    if(pcb->eof == 1) {
      pthread_mutex_unlock(&(pcb->win.win_lck));
      break;
    }
    
    pthread_mutex_unlock(&(pcb->win.win_lck));
  }
  return;
}
