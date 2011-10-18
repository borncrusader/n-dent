#ifndef __P2MP_H
#define __P2MP_H

#include <stdio.h>
#include <string.h>

#include <netinet/in.h>

#define FILE_NSIZE  100               // File name size
#define MSS         1500              // Maximum segment size of a packet
#define MAX_RECV    10                // Maximum number of receivers

#define BUF_TIMEOUT 2

#define P2MP_ZERO(p) memset(&(p), 0, sizeof(p))

typedef struct stats {
  int num_acks;                       // Number of acks excluding the dup acks
  int num_sent;                       // Number of pkts sent so far
  int dup_acks;                       // Number of duplicate acks
  int retransmissions;                // Number of fast retransmits
  time_t rtt[MAX_RECV];               // RTT of each connection
  pthread_mutex_t st_lck;             // Lock for statistics structure
}stats;

typedef struct node {
  int filled;                         // Set when the node is filled with data
  int acks[MAX_RECV];                 // used by receiver
  char buf[MSS];                      // place to store data
  struct node *next;                  // pointer to next node
}node;

typedef struct {
  int num_unack;                      // Last unacked seq number
  int num_avail;                      // Number of nodes filled with data
  int num_empty;                      // free nodes in the window

  node *head;                         // Head of the window
  node *left;                         // left boundary of window
  node *right;                        // right boundary of window
  node *tosend;                       // starting pointer of num_avail

  pthread_mutex_t win_lck;            // lock for window
  pthread_cond_t win_cnd;             // condition variable for window

}window;

typedef struct {
  int eof;                            // Set when reached end of file
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
