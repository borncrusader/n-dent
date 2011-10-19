#ifndef __P2MPCLIENT_H__
#define __P2MPCLIENT_H__

#include <stdio.h>
#include <string.h>

#include <netinet/in.h>

#include "p2mp.h"

typedef struct stats {
  int num_acks;                       // Number of acks excluding the dup acks
  int num_sent;                       // Number of pkts sent so far
  int dup_acks;                       // Number of duplicate acks
  int retransmissions;                // Number of fast retransmits
  time_t rtt[MAX_RECV];               // RTT of each connection
  pthread_mutex_t st_lck;             // Lock for statistics structure
}stats;

typedef struct node {
  int eof;                            // Set when reached end of file
  int seq_num;                        // sequence number; 0 when filled is 0
  int filled;                         // Set when the node is filled with data
  int acks[MAX_RECV];                 // used by receiver
  int buf_size;                       // number of bytes in the buffer
  char buf[BUFFER_SIZE];              // place to store data
  struct node *next;                  // pointer to next node
}node;

typedef struct {
  int num_unack;                      // Last unacked seq number
  int num_avail;                      // Number of nodes filled with data
  int num_empty;                      // free nodes in the window

  int data_available;                 // the aggregator thread sets this if data is available

  node *head;                         // Head of the window
  node *left;                         // left boundary of window
  node *right;                        // right boundary of window
  node *tosend;                       // starting pointer of num_avail

  pthread_mutex_t win_lck;            // lock for window

}window;

typedef struct {
  int N;                              // Windows size
  int mss;                            // Maximum segment size
  int num_recv;                       // Number of receivers
  int sockfd;                         // Socket fds connecting with the receiver
  char filename[FILE_NSIZE];          // file to send
  struct sockaddr_in recv[MAX_RECV];  // receivers
  stats cli_stats;                    // Stats structure
  window win;                         // window
  pthread_mutex_t pcb_lck;            // a lock for the structure
  pthread_t sender;                   // thread id of the send thread
  pthread_t receiver;                 // thread id of the receive thread
  pthread_t buf_mgr;                  // thread id of the buffer manager
}p2mp_pcb;

void* rdt_send(void*);
void* sender(void*);
void* receiver(void*);

#endif
