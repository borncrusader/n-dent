#include "p2mp.h"
#include "p2mpclient.h"

void* sender(void *args) {
  char ch;
  int ret = 0, i = 0, looper = 1;
  unsigned int seq_num = 0;

  struct itimerspec its;

  p2mp_pcb *pcb = (p2mp_pcb*)args;
  node *node_ptr = NULL;

  its.it_value.tv_sec = ACK_TIMEOUT;
  its.it_value.tv_nsec = 0;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  while(looper) {

    if(pcb->win.data_available) {
      pthread_mutex_lock(&(pcb->win.win_lck));
      node_ptr = pcb->win.to_send;
      while(node_ptr != NULL && node_ptr->filled == 1) {
        node_ptr->seq_num = seq_num;

        pack_data(seq_num, MSG_TYPE_DATA, node_ptr->flags, node_ptr->buf, node_ptr->buf_size);

        for(i=0;i<pcb->num_recv;i++) {
          printf("SENDER : Sending packet:%d to receiver:%d\n", seq_num, i);
          ret = sendto(pcb->sockfd,
                       node_ptr->buf,
                       node_ptr->buf_size,
                       0,
                       (struct sockaddr*)(&(pcb->recv[i])),
                       sizeof(struct sockaddr_in));
          if(ret==-1) {
            warn("SENDER : sento() failed! : ", errno);
          }
        }
        if(node_ptr->flags & FLAG_EOM) {
          printf("SENDER : EOM reached!\n");
          looper = 0;
        }

        // start the timer
        if(node_ptr->seq_num == pcb->win.head->seq_num) {
          timer_settime(pcb->timerid, 0, &its, NULL);
        }
        seq_num++;
        node_ptr = node_ptr->next;
      }
      pcb->win.to_send = node_ptr;

      pcb->win.data_available = 0;
      pthread_mutex_unlock(&(pcb->win.win_lck));
    } else {
      printf("SENDER : Waiting for data!\n");
      sleep(1);
    }
  }

  return;
}
