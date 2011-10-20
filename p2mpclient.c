#include "p2mp.h"
#include "p2mpclient.h"

void usage()
{
  printf("p2mpclient server-1 server-1-port [server-2 server-2-port ... server-r server-r-port] file-name N MSS\n");
  printf("  server-n      - server-n's ip address\n");
  printf("  server-n-port - server-n's port (at least 1 server should be specified, max 10)\n");
  printf("  file-name     - file to be transferred\n");
  printf("  N             - window size\n");
  printf("  MSS           - maximum segment size of each segment (< 1500)\n");
  exit(1);
}

int main(int argc, char *argv[])
{
  int i, st = 0;
  char *cfg_file = NULL;

  p2mp_pcb pcb;

  if(argc==1) {
    usage();
  }

  P2MP_ZERO(pcb);

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
    if(atoi(argv[argc-1]) < 0 || atoi(argv[argc-1]) > BUFFER_SIZE) {
      usage();
    }
    pcb.mss = atoi(argv[argc-1]);
    pcb.N = atoi(argv[argc-2]);
    strncpy(pcb.filename, argv[argc-3], FILE_NSIZE);

    for(i = 1 ; i < argc-3 && st <= MAX_RECV ; i+=2) {
      pcb.recv[st].sin_family = AF_INET;
      pcb.recv[st].sin_addr.s_addr = inet_addr(argv[i]);
      pcb.recv[st].sin_port = htons(atoi(argv[i+1]));
      ++st;
    }

    pcb.num_recv = st;
  }

  pcb.win.num_empty = pcb.N;

  pthread_create(&(pcb.buf_mgr), NULL, rdt_send, &pcb);
  pthread_create(&(pcb.sender), NULL, sender, &pcb);
  pthread_create(&(pcb.receiver), NULL, receiver, &pcb);

  pthread_join(pcb.buf_mgr, NULL);
  pthread_join(pcb.sender, NULL);
  pthread_join(pcb.receiver, NULL);

  return 0;
}
