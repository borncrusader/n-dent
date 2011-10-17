#ifndef __STAT_H__
#define __STAT_H__

typedef struct
{
  unsigned long pkts_rcvd;  /* number of packets (datagrams) received */
  unsigned long bytes_rcvd; /* number of bytes received in all datagrams */
  unsigned long ack_rcvd;   /* number of acks received */

  unsigned long pkts_sent; /* number of packets sent */
  unsigned long bytes_sent; /* number of bytes sent */
  unsigned long ack_sent; /* number of acks sent */

  unsigned long pkts_drop; /* number of packets dropped because of probability */
  unsigned long pkts_wrong; /* number of packets received wrongly - checksum failed! */
}stat_t;

#define INCREMENT_STAT(_s,_val,_step) \
  (_s)->_val+=_step

#define INITIALISE_STAT(_s) \
  memset(_s,0,sizeof(stat_t))

#endif
