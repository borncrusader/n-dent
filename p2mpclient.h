#ifndef __P2MPCLIENT_H__
#define __P2MPCLIENT_H__

typedef struct
{
  char **servers;
  char *filename;
  int num_servers;
  int s_port;
  int win_size;
  int mss;
  pthread_t timer_thread;
}client_t;

void cleanup(client_t*);

#endif
