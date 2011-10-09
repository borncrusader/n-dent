#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "p2mpclient.h"

void usage()
{
  printf("p2mpclient server-1 [server-2 ... server-r] server-port file-name N MSS\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int i;
  char *cfg_file = NULL;
  client_t client;

  if(argc==1) {
    usage();
  }

  if(!strncmp(argv[1], "-f", 3)) {
    if (argc != 3) {
      usage();
    }
    cfg_file = argv[2];
  } else if(!strncmp(argv[1], "-h", 3)) {
    usage();
  } else {
    if(argc < 6) {
      usage();
    }
    client.mss = atoi(argv[argc-1]);
    client.win_size = atoi(argv[argc-2]);
    client.filename = argv[argc-3];
    client.s_port = atoi(argv[argc-4]);

    client.num_servers = argc-5;
    client.servers = argv + 1;
  }

  return 0;
}
