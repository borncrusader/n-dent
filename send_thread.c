#include <stdio.h>
#include <pthread.h>

#include "p2mp.h"

void* sender(void *args) {
  int i = 0;
  unsigned char looper = 1;
  struct timespec s_timer;
  p2mp_pcb *pcb = (p2mp_pcb*)args;

  s_timer.tv_sec = 2;
  s_timer.tv_nsec = 0;

  for(i = 0 ; i < pcb->num_recv ; ++i)
    pcb->sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);

  while(looper) {
    pthread_mutex_lock(&(pcb->win.win_lck));

    pthread_cond_timedwait(&(pcb->win.win_cnd), &(pcb->win.win_lck), &s_timer);

    if(pcb->eof == 1) {
      pthread_mutex_unlock(&(pcb->win.win_lck));
      break;
    }
    
    pthread_mutex_unlock(&(pcb->win.win_lck));
  }
  return;
}
