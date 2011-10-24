#include "p2mp.h"
#include "p2mpclient.h"

void* timer(void *args)
{
  p2mp_pcb *pcb = (p2mp_pcb*)args;
  int i = 0, pos = 0, ret = 0;

  pthread_mutex_lock(&(pcb->win.win_lck));

  printf("TIMER : TIMED OUT");

  if(pcb->win.head->filled) {
    for(pos=0; pos<pcb->num_recv; pos++) {
      if(pcb->win.head->acks[pos] == 0) {
        printf("TIMER: Sending seq_num : %d to %s\n", pcb->win.head->seq_num, inet_ntoa(pcb->recv[pos].sin_addr));
        ret = sendto(pcb->sockfd,
                     pcb->win.head->buf,
                     pcb->win.head->buf_size,
                     0,
                     (struct sockaddr*)(&(pcb->recv[pos])),
                     sizeof(struct sockaddr_in));
        if(ret==-1) {
          warn("TIMER: sento() failed! : ", errno);
        }
      }
    }
  }

  pthread_mutex_unlock(&(pcb->win.win_lck));

}

/*
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
*/
