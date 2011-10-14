#ifndef __P2MP_H
#define __P2MP_H

#include <netinet/in.h>

#define FILE_NSIZE  100
#define MSS         1500
#define MAX_RECV    10

#define P2MP_ZERO(p) memset(&p, 0, sizeof(p))

typedef struct {
  int num_acks;                       // Number of acks excluding the dup acks
  int num_sent;                       // Number of pkts sent so far
  int dup_acks;                       // Number of duplicate acks
  int retransmissions;                // Number of fast retransmits
  time_t rtt[MAX_RECV];               // RTT of each connection
}stats;

typedef struct {
  int acks[MAX_RECV];                 // used by receiver
  char buf[MSS];                      // place to store data
  struct window *next;                // pointer to next node
}window;

typedef struct {
  int eof;                            // Set when reached end of file
  int N;                              // Windows size
  int mss;                            // Maximum segment size
  int unack;                          // Last unacked seq number
  int avail;                          // Number of nodes filled with data
  int empty;                          // free nodes in the window
  int num_recv;                       // Number of receivers
  int sockfd[MAX_RECV];               // Socket fds connecting with the receiver
  char filename[FILE_NSIZE];          // file to send
  struct sockaddr_in recv[MAX_RECV];  // receivers
  stats cli_stats;                    // Stats structure
  window *head;                       // Head of window
  window *left;                       // left boundary of window
  window *right;                      // right boundary of window
  window *tosend;                     // yet to send data
  pthread_mutex_t lck;                // a lock for the structure
  pthread_t sender;                   // thread id of the send thread
  pthread_t receiver;                 // thread id of the receive thread
  pthread_t buf_mgr;                  // thread id of the buffer manager
}p2mp_pcb;

void* rdt_send(void*);
void* sender(void*);
void* receiver(void*);

#endif
