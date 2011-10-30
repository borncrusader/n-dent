#include "p2mp.h"
#include "p2mpclient.h"

void* receiver(void *args) {

  int ret = 0, seq_num = 0, type = 0, flags = 0, rcvd_size = 0;
  int pos = 0, ser_pos = 0, brk = 0, diff_seq_num = 0;
  int dup_ack[MAX_RECV];
  char buf[BUFFER_SIZE];
  char from[INET_ADDRSTRLEN];
  struct sockaddr_in ser;
  unsigned char looper = 1;
  node *node_ptr, *node_temp;
  socklen_t len;

  p2mp_pcb *pcb = (p2mp_pcb *)args;

  len = sizeof(ser);

  while(looper) {

    ret = recvfrom(pcb->sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&ser, &len);

    inet_ntop(AF_INET, &(ser.sin_addr), from, INET_ADDRSTRLEN);
    from[INET_ADDRSTRLEN-1] = '\0';

    if(ret == 0) { // Will this return value ever come ???
      warn("RECEIVER : Received a 0 return value", errno);
      // Code to remove the receiver from the p2mp_pcb structure
      continue;
    } else if(ret == -1) {
      warn("RECEIVER : recvfrom() error", errno);
      continue;
    }

    if(unpack_data(&seq_num, &type, &flags, buf, ret) == -1) {
      warn("RECEIVER : Checksum error", 0);
      continue;
    }

    if(type != MSG_TYPE_ACK) {
      warn("RECEIVER : Received a dubious packet", 0);
      continue;
    }

    rcvd_size = ret;

    printf("RECEIVER : Received ack from %s:%d, sequence number = %d\n", from, ntohs(ser.sin_port), seq_num);

    pos = 0;
    while(pos < pcb->num_recv) {
      if(pcb->recv[pos].sin_addr.s_addr == ser.sin_addr.s_addr && 
          pcb->recv[pos].sin_port == ser.sin_port) {
        ser_pos = pos;
        break;
      }
      ++pos;
    }

    if(pos == pcb->num_recv) {
      warn("RECEIVER : Received packet from unknown server", 0);
      continue;
    }

    pthread_mutex_lock(&(pcb->win.win_lck));

    node_ptr = pcb->win.head;

    if(seq_num == node_ptr->seq_num-1) {
      dup_ack[ser_pos]++;

      P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_DUPACKS_RCVD, ser_pos);
      P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_DUPACKS_BYTES_RCVD, ser_pos, rcvd_size);

      if(dup_ack[ser_pos] >= 2) {

        // Fast Retransmit code flow
        //printf("RECEIVER : Num of dup_acks : %d for seq_num : %d\n", dup_ack[ser_pos], seq_num);

        dup_ack[ser_pos] = 0;

        for(pos = 0 ; pos < pcb->num_recv ; ++pos) {
          if(node_ptr->acks[pos] == 0) {

            printf("RECEIVER : Fast Retransmit to %s:%d, sequence number = %d\n", inet_ntoa(pcb->recv[pos].sin_addr),
                   ntohs(pcb->recv[pos].sin_port), node_ptr->seq_num);

            ret = sendto(pcb->sockfd,
                node_ptr->buf,
                node_ptr->buf_size,
                0,
                (struct sockaddr *)&(pcb->recv[pos]),
                sizeof(pcb->recv[pos]));

            if(ret==-1) {
              warn("RECEIVER : sento() failed!", errno);
            } else {
              P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_FRTRANS_PKTS_SENT, pos);
              P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_FRTRANS_BYTES_SENT, pos, ret);
            }
          }
        }
      }
    }

    diff_seq_num = seq_num - pcb->win.head->seq_num;

    node_ptr = pcb->win.head;
    while(node_ptr &&
       node_ptr->seq_num > -1 &&
       diff_seq_num >= 0) {

      ++(node_ptr->acks[ser_pos]);

      if(seq_num == node_ptr->seq_num) {
        if(node_ptr->acks[ser_pos] == 1) {
          P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_ACKS_RCVD, pos);
          P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_ACKS_BYTES_RCVD, pos, rcvd_size);
        } else {
          P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_DUPACKS_RCVD, pos);
          P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_DUPACKS_BYTES_RCVD, pos, rcvd_size);
        }
      }

      if(node_ptr->acks[ser_pos] > 2) {
        // Fast Retransmit code
        // Resend next packet to all servers from which we have not received ACK
        node_ptr->acks[ser_pos] = 1;
        for(pos = 0 ; pos < pcb->num_recv ; ++pos) {
          if(node_ptr->next && node_ptr->next->acks[pos] == 0) {

            printf("RECEIVER : Fast Retransmit to %s:%d, sequence number = %d\n", inet_ntoa(pcb->recv[pos].sin_addr),
                   ntohs(pcb->recv[pos].sin_port), node_ptr->seq_num);

            ret = sendto(pcb->sockfd,
                    node_ptr->next->buf,
                    node_ptr->next->buf_size,
                    0,
                    (struct sockaddr *)&(pcb->recv[pos]),
                    sizeof(pcb->recv[pos]));
            if(ret==-1) {
              warn("RECEIVER : sento() failed!", errno);
            } else {
              P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_FRTRANS_PKTS_SENT, pos);
              P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_FRTRANS_BYTES_SENT, pos, ret);
            }
          }
        }
      }
      node_ptr = node_ptr->next;
      diff_seq_num--;
    }

    diff_seq_num = seq_num - pcb->win.head->seq_num;

    brk = 0;
    node_ptr = pcb->win.head;
    while(node_ptr &&
       node_ptr->seq_num > -1 &&
       diff_seq_num >= 0) {

      for(pos = 0; pos<pcb->num_recv; pos++) {
        if(node_ptr->acks[pos] == 0) {
          brk = 1;
          break;
        }
      }
      if(brk == 1) {
        break;
      }

      if(node_ptr->seq_num == pcb->win.head->seq_num) {
        if(pcb->win.last_seq == node_ptr->seq_num) {
          looper = 0;
        }
        else {
          //printf("RECEIVER : Resetting timer\n");
          timer_settime(pcb->timerid, 0, &(pcb->timer_val), NULL);
        }
      }

      node_temp = node_ptr->next;

      P2MP_ZERO_ADDR(node_ptr, struct node);
      P2MP_ZERO(dup_ack);
      node_ptr->seq_num = -1;

      pcb->win.tail->next = node_ptr;
      pcb->win.tail = node_ptr;
      if(node_temp != NULL)
        pcb->win.head = node_temp;
      else {
        pcb->win.head = pcb->win.tail;
        pcb->win.tail->next = NULL;
      }
      node_ptr = pcb->win.head;
      (pcb->win.num_empty)++;
      diff_seq_num--;

      printf("RECEIVER : Num of empty nodes in window : %d\n", pcb->win.num_empty);

    }

    pthread_mutex_unlock(&(pcb->win.win_lck));
  }

  //printf("RECEIVER : receive_thread terminating\n");
  return;
}
