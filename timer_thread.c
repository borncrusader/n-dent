#include "p2mp.h"
#include "p2mpclient.h"

void* timer(void *args)
{
  p2mp_pcb *pcb = (p2mp_pcb*)args;
  int looper = 1;

  while(looper) {
    sleep(1);

    pthread_mutex_lock(&(pcb->win.win_lck));
    if(pcb->win.head->filled) {
      pcb->win.timer--;
      if(pcb->win.timer == 0) {
        for(pos=0; pos<pcb->num_recv; pos++) {
          if(pcb->win.acks[pos] == 0) {
            ret = sendto(pcb->sockfd,
                         pcb->win.head->buf,
                         pcb->win.head->buf_size,
                         0,
                         (struct sockaddr*)(&(pcb->recv[i])),
                         sizeof(struct sockaddr_in));
            if(ret==-1) {
              warn("timer: sento() failed! : ", errno);
            }
          }
        }
      }
    }
    pthread_mutex_unlock(&(pcb->win.win_lck));
  }
}
