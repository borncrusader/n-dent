#include <stdio.h>
#include <errno.h>

#include "p2mp.h"
#include "p2mpclient.h"

void* receiver(void *args) {
  int ret = 0, seq_num = 0, type = 0, flags = 0;
  int pos = 0, ser_pos = 0;
  char buf[MSS];
  char from[INET_ADDRSTRLEN];
  struct sockaddr_in ser;
  unsigned char looper = 1;
  node *win_ptr = NULL, *win_re_ptr = NULL;
  socklen_t len;

  p2mp_pcb *pcb = (p2mp_pcb *)args;

  printf("receiver: waiting for acks..\n");

  while(looper) {
    ret = recvfrom(pcb->sockfd, buf, MSS-1, 0, (struct sockaddr*)&ser, &len);

    inet_ntop(AF_INET, &ser, from, INET_ADDRSTRLEN);
    from[INET_ADDRSTRLEN] = '\0';

    printf("receiver: Received msg from %s\n", from);

    if(ret == 0) { // Will this return value ever come ???
      warn("receiver: received a 0 return value : ", errno);
      // Code to remove the receiver from the p2mp_pcb structure
      continue;
    }

    if(ret == -1) {
      warn("receiver: recvfrom() error : ", errno);
      continue;
    }

    if(unpack_data(&seq_num, &type, &flags, buf, ret) == -1) {
      warn("receiver: Checksum error", 0);
      continue;
    }

    if(type != MSG_TYPE_ACK) {
      warn("receiver: received a dubious packet", 0);
      continue;
    }

    pthread_mutex_lock(&(pcb->pcb_lck));
    pos = 0;
    while(pos < MAX_RECV) {
      if(pcb->recv[pos].sin_addr.s_addr == ser.sin_addr.s_addr) {
        ser_pos = pos;
        break;
      }
      ++pos;
    }
    pthread_mutex_unlock(&(pcb->pcb_lck));

    if(pos == MAX_RECV) {
      warn("receiver: received packet from unknown server", 0);
      continue;
    }

    pthread_mutex_lock(&(pcb->win.win_lck));

    win_ptr = pcb->win.head;

    while(win_ptr) {

      if(win_ptr->seq_num == seq_num) {
        ++(win_ptr->acks[ser_pos]);
        if(win_ptr->acks[ser_pos] > 2) {
          // Fast Retransmit code
          // Resend packet to all servers from which we have not received any ACK
          flags = 0;
          if(win_ptr->eof == 1) {
            flags = FLAG_EOM;
          }

          pos = 0;
          while(pos < pcb->num_recv) {
            if(win_ptr->acks[pos] == 0 ||
                win_ptr->acks[pos] > 2) {

              // pack_data needs to return a buf containing msg+header
              // pack_data(win_ptr->seq_num, MSG_TYPE_DATA, flags, win_ptr->buf, pcb->mss+HEADER_SIZE);

              sendto(pcb->sock_fd, buffer, pcb->mss+HEADER_SIZE, 0, (struct sockaddr *)&(pcb->recv[pos]), sizeof(pcb->recv[pos]));
            }
          }

        }
        else
          break;
      }
      win_ptr = win_ptr->next;
    }

  }
  return;
}
