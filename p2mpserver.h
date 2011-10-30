#ifndef __P2MPSERVER_H__
#define __P2MPSERVER_H__

#include "p2mp.h"

typedef enum {
  P2MPS_STAT_ACKS_SENT = 0,         // number of ack packets sent
  P2MPS_STAT_ACKS_BYTES_SENT,       // number of ack bytes sent
  P2MPS_STAT_DUPACKS_SENT,          // number of dupack packets sent
  P2MPS_STAT_DUPACKS_BYTES_SENT,    // number of dupack bytes sent

  P2MPS_STAT_PKTS_IS_RCVD,          // number of packets received in-sequence
  P2MPS_STAT_BYTES_IS_RCVD,         // number of bytes received in-sequence
  P2MPS_STAT_PKTS_OS_RCVD,          // number of packets received out-of-sequence
  P2MPS_STAT_BYTES_OS_RCVD,         // number of bytes received out-of-sequence
  P2MPS_STAT_PKTS_NR_RCVD,          // number of packets received which are already buffered
  P2MPS_STAT_BYTES_NR_RCVD,         // number of bytes received which are already buffered

  P2MPS_STAT_PKTS_CORRUPT,          // number of packets received incorrectly
  P2MPS_STAT_BYTES_CORRUPT,         // number of bytes received incorrectly

  P2MPS_STAT_PKTS_DROP,             // number of packets dropped
  P2MPS_STAT_BYTES_DROP,            // number of bytes dropped

  P2MPS_STAT_MAX
}p2mps_stat_type;

/*
typedef struct rev_window {
  struct node *left;
  struct node *right;
  struct node *head;
  struct node *toack;
} rev_window;
   */

typedef struct p2mp_sb {
  float p;
  int N;
  int sock_server_recv;
  char filename[FILE_NSIZE];
  //struct rev_window w;
  unsigned long stat[P2MPS_STAT_MAX];
  char md5[MD5_LEN+1];
} p2mp_sb;

typedef struct node {
	int seqnum;
	int filled;
	int ack;
  int buf_size;
	char buf[BUFFER_SIZE];
	//struct node *next;
}node;

#define P2MPS_STAT_INCREMENT(_s, _t) \
  (_s)->stat[_t]++;

#define P2MPS_STAT_UPDATE(_s, _t, _b) \
  (_s)->stat[_t] += _b;

#endif
