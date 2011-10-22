#include "p2mp.h"
#include "p2mpclient.h"

sig_atomic_t timer_expired;

void* timer(void *args)
{
  p2mp_pcb *pcb = (p2mp_pcb*)args;
  int i = 0, looper = 1, pos = 0, ret = 0;

  while(looper) {

    while(!timer_expired);

    pthread_mutex_lock(&(pcb->win.win_lck));
    if(pcb->win.head->filled) {
      for(pos=0; pos<pcb->num_recv; pos++) {
        if(pcb->win.head->acks[pos] == 0) {
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
    pthread_mutex_unlock(&(pcb->win.win_lck));

    timer_expired = 0;
  }
}

void timer_callback(int sig)
{
  timer_expired = 1;
  
  return;
}

void timer_start(int secs)
{
  signal(SIGALRM, timer_callback);
  alarm(secs);

  return;
}

void timer_stop()
{
  signal(SIGALRM, SIG_IGN);

  return;
}
