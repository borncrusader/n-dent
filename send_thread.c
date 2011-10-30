#include "p2mp.h"
#include "p2mpclient.h"

void* sender(void *args) {

  char ch;
  int ret = 0, i = 0, looper = 1;
  unsigned int seq_num = 0;

  struct itimerspec its, now;

  p2mp_pcb *pcb = (p2mp_pcb*)args;
  node *node_ptr = NULL;

  pcb->win.last_seq = -1;

  its.it_value.tv_sec = ACK_SEC;
  its.it_value.tv_nsec = ACK_NSEC;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 0;

  while(looper) {

    //timer_gettime(pcb->timerid, &now);
    //printf("current timer value %ld %ld %ld %ld\n", now.it_value.tv_sec, now.it_value.tv_nsec, now.it_interval.tv_sec, now.it_interval.tv_nsec);

    pthread_mutex_lock(&(pcb->win.win_lck));

    if(pcb->win.data_available) {

      node_ptr = pcb->win.to_send;
      while(node_ptr != NULL && node_ptr->filled == 1) {
        node_ptr->seq_num = seq_num;

        pack_data(seq_num, MSG_TYPE_DATA, node_ptr->flags, node_ptr->buf, node_ptr->buf_size);

        for(i=0;i<pcb->num_recv;i++) {
          printf("SENDER : Sending packet, sequence number = %d to receiver = %s:%d\n", seq_num, inet_ntoa(pcb->recv[i].sin_addr), ntohs(pcb->recv[i].sin_port));
          ret = sendto(pcb->sockfd,
                       node_ptr->buf,
                       node_ptr->buf_size,
                       0,
                       (struct sockaddr*)(&(pcb->recv[i])),
                       sizeof(struct sockaddr_in));
          if(ret==-1) {
            warn("SENDER : sento() failed!", errno);
          } else {
            P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_PKTS_SENT, i);
            P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_BYTES_SENT, i, node_ptr->buf_size);
          }
        }
        if(node_ptr->flags & FLAG_EOM) {
          //printf("SENDER : EOM reached!\n");
          pcb->win.last_seq = seq_num;
          looper = 0;
        }

        // start the timer
        if(node_ptr->seq_num == pcb->win.head->seq_num) {
          //printf("SENDER : Starting timer for head packet\n");
          timer_settime(pcb->timerid, 0, &its, NULL);
        }
        seq_num++;
        node_ptr = node_ptr->next;
      }

      if(node_ptr != NULL)
        pcb->win.to_send = node_ptr;
      else
        pcb->win.to_send = pcb->win.head;

      pcb->win.data_available = 0;
    } else {
      //printf("SENDER : Waiting for data!\n");
      //sleep(1);
    }
    pthread_mutex_unlock(&(pcb->win.win_lck));
  }

  //printf("SENDER : send_thread terminating\n");

  return;
}
