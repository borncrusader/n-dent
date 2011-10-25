#ifndef __P2MPCLIENT_H__
#define __P2MPCLIENT_H__

#include "p2mp.h"

extern sig_atomic_t timer_expired;

typedef struct stats {
  unsigned long acks_rcvd[MAX_RECV];     // Number of acks excluding the dup acks
  unsigned long pkts_sent[MAX_RECV];     // Number of pkts sent so far
  unsigned long bytes_sent[MAX_RECV];    // Number of bytes sent so far
  unsigned long dupacks_rcvd[MAX_RECV];  // Number of duplicate acks
  unsigned long rtrans_sent[MAX_RECV];   // Number of fast retransmits
  time_t rtt[MAX_RECV];                  // RTT of each connection
  pthread_mutex_t st_lck;                // Lock for statistics structure
}stats;

typedef struct node {
  int flags;                          // Set when reached end of file
  int seq_num;                        // sequence number; 0 when filled is 0
  int filled;                         // Set when the node is filled with data
  int acks[MAX_RECV];                 // used by receiver
  int buf_size;                       // number of bytes in the buffer
  char buf[BUFFER_SIZE];              // place to store data
  struct node *next;                  // pointer to next node
}node;

typedef struct {
  int last_seq;                       // Last sequence number of packet
  int num_empty;                      // free nodes in the window

  int data_available;                 // the aggregator thread sets this if data is available

  node *head;                         // Head of the window
  node *tail;                         // tail of the window
  node *to_send;                      // starting pointer of num_avail

  pthread_mutex_t win_lck;            // lock for window

}window;

typedef struct {
  int N;                              // Windows size
  int mss;                            // Maximum segment size
  int num_recv;                       // Number of receivers
  int sockfd;                         // Socket fds connecting with the receiver
  int bind_port;                      // port to which the receiver should bind to
  char filename[FILE_NSIZE];          // file to send
  struct sockaddr_in recv[MAX_RECV];  // receivers
  stats cli_stats;                    // Stats structure
  window win;                         // window
  timer_t timerid;                    // Timer identifier
  pthread_t sender;                   // thread id of the send thread
  pthread_t receiver;                 // thread id of the receive thread
  pthread_t buf_mgr;                  // thread id of the buffer manager
  pthread_t timer;                    // thread id of the timer thread
}p2mp_pcb;

void* rdt_send(void*);
void* sender(void*);
void* receiver(void*);
void* timer(void*);
void timer_start(int);
void timer_stop();

#endif
