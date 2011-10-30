#ifndef __P2MPCLIENT_H__
#define __P2MPCLIENT_H__

#include "p2mp.h"

typedef enum {
  P2MPC_STAT_ACKS_RCVD = 0,          // number of acks received
  P2MPC_STAT_ACKS_BYTES_RCVD,        // number of acks in bytes received
  P2MPC_STAT_DUPACKS_RCVD,           // number of acks received
  P2MPC_STAT_DUPACKS_BYTES_RCVD,     // number of acks received
  P2MPC_STAT_FRTRANS_PKTS_SENT,      // number of acks received
  P2MPC_STAT_FRTRANS_BYTES_SENT,     // number of acks received

  P2MPC_STAT_PKTS_SENT,              // number of acks received
  P2MPC_STAT_BYTES_SENT,             // number of acks received

  P2MPC_STAT_TRTRANS_PKTS_SENT,      // number of acks received
  P2MPC_STAT_TRTRANS_BYTES_SENT,     // number of acks received
  P2MPC_STAT_TIMER_EXPIRED,          // number of acks received, only at recv = 0

  P2MPC_STAT_RDT_SEND_PKTS_READ,     // number of acks received, only at recv = 0
  P2MPC_STAT_RDT_SEND_BYTES_READ,    // number of acks received, only at recv = 0

  P2MPC_STAT_MAX
}p2mpc_stat_type;

typedef struct stats {
  unsigned long stat[P2MPC_STAT_MAX][MAX_RECV];
  time_t rtt[MAX_RECV];                  // RTT of each connection
  //pthread_mutex_t st_lck;                // Lock for statistics structure
}stats_t;

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
  stats_t stat;                       // Stats structure
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

#define P2MPC_STAT_INCREMENT(_s, _t, _r) \
  (_s)->stat[_t][_r]++;

#define P2MPC_STAT_UPDATE(_s, _t, _r, _b) \
  (_s)->stat[_t][_r] += _b;

#endif
