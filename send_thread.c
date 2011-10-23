#include "p2mp.h"
#include "p2mpclient.h"

void* sender(void *args) {
  int looper = 1;
  p2mp_pcb *pcb = (p2mp_pcb*)args;
  node *node_ptr = NULL;
  unsigned int seq_num = 0;
  int ret, i;
  char ch;

  while(looper) {

    if(pcb->win.data_available) {
      printf("server : Data available!\n");
      pthread_mutex_lock(&(pcb->win.win_lck));
      node_ptr = pcb->win.to_send;
      while(node_ptr != NULL && node_ptr->filled == 1) {
        node_ptr->seq_num = seq_num;

        pack_data(seq_num, MSG_TYPE_DATA, node_ptr->flags, node_ptr->buf, node_ptr->buf_size);

        /*i=0;
        while(i<node_ptr->buf_size) {
          ch = node_ptr->buf[i++];
          printf("%c", ch);
        }*/

        for(i=0;i<pcb->num_recv;i++) {
          printf("sending packet:%d to receiver:%d\n", seq_num, i);
          ret = sendto(pcb->sockfd,
                       node_ptr->buf,
                       node_ptr->buf_size,
                       0,
                       (struct sockaddr*)(&(pcb->recv[i])),
                       sizeof(struct sockaddr_in));
          if(ret==-1) {
            warn("sender: sento() failed! : ", errno);
          }
        }
        if(node_ptr->flags & FLAG_EOM) {
          printf("sender : EOM reached!\n");
          looper = 0;
        }

        // start the timer
        if(node_ptr == pcb->win.head) {
          timer_start(ACK_TIMEOUT);
        }
        seq_num++;
        node_ptr = node_ptr->next;
      }
      pcb->win.to_send = node_ptr;

      pcb->win.data_available = 0;
      pthread_mutex_unlock(&(pcb->win.win_lck));
    } else {
      printf("server : waiting for data!\n");
      sleep(1);
    }
  }

  return;
}
