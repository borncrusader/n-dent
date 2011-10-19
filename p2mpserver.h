#ifndef __P2MPSERVER_H__
#define __P2MPSERVER_H__

#include "p2mp.h"

typedef struct stats {
  unsigned long num_pkts_rcvd;        // number of packets received
  unsigned long num_bytes_rcvd;       // number of bytes received
  unsigned long num_acks_sent;        // number of acks sent excluding the dup acks
  unsigned long num_dup_acks_sent;    // number of dup acks sent
  unsigned long num_pkts_drop;        // number of packets dropped
  unsigned long num_pkts_wrong;       // number of packets corrupted
  pthread_mutex_t st_lck;             // Lock for statistics structure
}stats;

typedef struct rev_window {
        struct node *left;
        struct node *right;
        struct node *head;
        struct node *toack;
} rev_window;

typedef struct p2mp_sb {
	int p;
	int N;
	int sock_server;
	char filename[FILE_NSIZE];
	struct sockaddr_in server;
	struct rev_window w;
} p2mp_sb;


typedef struct node {
	int seqnum;
	int filled;
	int ack;
	char buf[BUFFER_SIZE];
	struct node *next;
}node;

#endif
