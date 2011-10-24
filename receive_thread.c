#include "p2mp.h"
#include "p2mpclient.h"

void* receiver(void *args) {
  int ret = 0, seq_num = 0, type = 0, flags = 0;
  int pos = 0, ser_pos = 0, brk = 0, diff_seq_num = 0;
  int dup_ack[MAX_RECV];
  char buf[BUFFER_SIZE];
  char from[INET_ADDRSTRLEN];
  struct sockaddr_in ser;
  unsigned char looper = 1;
  node *node_ptr = NULL;
  node *node_temp = NULL;
  socklen_t len;

  p2mp_pcb *pcb = (p2mp_pcb *)args;

  len = sizeof(ser);

  printf("receiver: waiting for acks..\n");

  while(looper) {
    ret = recvfrom(pcb->sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr*)&ser, &len);

    inet_ntop(AF_INET, &ser, from, INET_ADDRSTRLEN);
    from[INET_ADDRSTRLEN-1] = '\0';

    if(ret == 0) { // Will this return value ever come ???
      warn("receiver: received a 0 return value : ", errno);
      // Code to remove the receiver from the p2mp_pcb structure
      continue;
    } else if(ret == -1) {
      warn("receiver: recvfrom() error : ", errno);
      continue;
    }

    if(unpack_data(&seq_num, &type, &flags, buf, ret) == -1) {
      warn("receiver: Checksum error", 0);
      continue;
    }

    printf("receiver: Received msg from %s with seq_num %d len %d\n", from, seq_num, ret);

    if(type != MSG_TYPE_ACK) {
      warn("receiver: received a dubious packet", 0);
      continue;
    }

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
      warn("receiver: received packet from unknown server", 0);
      continue;
    }

    pthread_mutex_lock(&(pcb->cli_stats.st_lck));
    pcb->cli_stats.acks_rcvd[ser_pos]++;
    pthread_mutex_unlock(&(pcb->cli_stats.st_lck));

    pthread_mutex_lock(&(pcb->win.win_lck));

    node_ptr = pcb->win.head;

    if(seq_num == node_ptr->seq_num-1) {
      dup_ack[ser_pos]++;
      if(dup_ack[ser_pos] >= 2) {

        for(pos = 0 ; pos < pcb->num_recv ; ++pos) {
          if(node_ptr->acks[pos] == 0) {
            sendto(pcb->sockfd,
                node_ptr->buf,
                node_ptr->buf_size,
                0,
                (struct sockaddr *)&(pcb->recv[pos]),
                sizeof(pcb->recv[pos]));
          }
        }

      }
    }

    while(node_ptr) {

      if(node_ptr->seq_num <= seq_num) {
        ++(node_ptr->acks[ser_pos]);
        if(node_ptr->acks[ser_pos] > 2) {
          // Fast Retransmit code
          // Resend next packet to all servers from which we have not received ACK
          for(pos = 0 ; pos < pcb->num_recv ; ++pos) {
            if(node_ptr->next->acks[pos] == 0) {

              sendto(pcb->sockfd,
                  node_ptr->next->buf,
                  node_ptr->next->buf_size,
                  0,
                  (struct sockaddr *)&(pcb->recv[pos]),
                  sizeof(pcb->recv[pos]));
            }
          }
        }
      }
      else {
        break;
      }
      node_ptr = node_ptr->next;
    }

    diff_seq_num = seq_num - pcb->win.head->seq_num;

    brk = 0;
    node_ptr = pcb->win.head;
    while(node_ptr && diff_seq_num >= 0) {
      printf("receiver: node_seq_num:%d recv_seq_num:%d\n", node_ptr->seq_num, seq_num);
      for(pos = 0; pos<pcb->num_recv; pos++) {
        if(node_ptr->acks[pos] == 0) {
          brk = 1;
          break;
        }
      }
      if(brk == 1) {
        break;
      }

      if(node_ptr == pcb->win.head) {
        timer_stop();
      }

      node_temp = node_ptr->next;

      P2MP_ZERO_ADDR(node_ptr, struct node);
      P2MP_ZERO(dup_ack);

      pcb->win.tail->next = node_ptr;
      pcb->win.tail = node_ptr;
      pcb->win.head = node_temp;
      node_ptr = pcb->win.head;
      printf("receiver: Num of empty nodes in window : %d\n", pcb->win.num_empty);
      (pcb->win.num_empty)++;
      diff_seq_num--;
    }

    pthread_mutex_unlock(&(pcb->win.win_lck));
  }

  return;
}
