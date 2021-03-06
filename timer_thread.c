#include "p2mp.h"
#include "p2mpclient.h"

void* timer(void *args)
{
  int i = 0, pos = 0, ret = 0;
  struct itimerspec its;

  p2mp_pcb *pcb = (p2mp_pcb*)args;

  its.it_value.tv_sec = ACK_SEC;
  its.it_value.tv_nsec = ACK_NSEC;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  pthread_mutex_lock(&(pcb->win.win_lck));

  //printf("TIMER : TIMED OUT\n");
  P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_TIMER_EXPIRED, 0);

  if(pcb->win.head->filled) {
    for(pos=0; pos<pcb->num_recv; pos++) {
      if(pcb->win.head->acks[pos] == 0) {
        ret = sendto(pcb->sockfd,
                     pcb->win.head->buf,
                     pcb->win.head->buf_size,
                     0,
                     (struct sockaddr*)(&(pcb->recv[pos])),
                     sizeof(struct sockaddr_in));
        if(ret==-1) {
          warn("TIMER : sento() failed!", errno);
        } else {
          P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_TRTRANS_PKTS_SENT, pos);
          P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_TRTRANS_BYTES_SENT, pos, pcb->win.head->buf_size);
        }
        printf("\033[22;31mTIMER : Timeout, sequence number = %d to %s:%d\n", pcb->win.head->seq_num,
               inet_ntoa(pcb->recv[pos].sin_addr), ntohs(pcb->recv[pos].sin_port));
	printf("%c[%dm", 0x1B, 0);

      }
    }
  }

  timer_settime(pcb->timerid, 0, &its, NULL);

  pthread_mutex_unlock(&(pcb->win.win_lck));
}
