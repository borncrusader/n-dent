#ifdef __P2MP_H
#define __P2MP_H

struct buffer {
  int eof;                    // Set when reached end of file
  int N;                      // Windows size
  int unack;                  // Last unacked seq number
  int avail;                  // Number of nodes filled with data
  int empty;                  // free nodes in the window
  struct stats cli_stat;      // Stats structure
  struct window *head;        // Head of window
  struct window *left;        // left boundary of window
  struct window *right;       // right boundary of window
  struct window *tosend;      // yet to send data
  pthread_mutext_lock lck;    // a lock for the structure
  int sockfd[MAX_CLIENTS];    // Socket fds connecting with the receiver
  struct sockaddr_in recv[NUM_CLIENTS];
};

struct stats {
  int num_acks;               // Number of acks excluding the dup acks
  int num_sent;               // Number of pkts sent so far
  int dup_acks;               // Number of duplicate acks
  int retransmissions;        // Number of fast retransmits
  time_t rtt[NUM_CLIENTS];    // RTT of each connection
};

struct window {
  int acks[NUM_CLIENTS];      // used by receiver
  char buf[MSS];              // place to store data
  struct window *next;        // pointer to next node
};

#endif
