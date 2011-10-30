#include "p2mp.h"
#include "p2mpclient.h"

void usage()
{
  printf("p2mpclient server-1 server-1-port [server-2 server-2-port ... server-r server-r-port] file-name N MSS\n");
  printf("           server-n      - server-n's ip address (at least 1 server should be specified, max 10)\n");
  printf("           server-n-port - server-n's port (at least 1 server should be specified, max 10)\n");
  printf("           file-name     - file to be transferred\n");
  printf("           N             - window size\n");
  printf("           MSS           - maximum segment size of each segment (< 1500)\n");
  exit(1);
}

void print_stats(p2mp_pcb *pcb)
{
  stats_t *s = &(pcb->stat);
  int i;
  struct timeval diff;

  printf("\nGlobal stats\n");
  printf("Timer expired         : %ld\n"
         "RDT Send pkts read    : %ld\n"
         "RDT Send bytes read   : %ld\n",
         s->stat[P2MPC_STAT_TIMER_EXPIRED][0],
         s->stat[P2MPC_STAT_RDT_SEND_PKTS_READ][0],
         s->stat[P2MPC_STAT_RDT_SEND_BYTES_READ][0]);

  for(i=0; i<pcb->num_recv; i++) {
    printf("\nReceiver %d, %s:%d\n", i,
           inet_ntoa(pcb->recv[i].sin_addr),
           ntohs(pcb->recv[i].sin_port));
    printf("Packets Sent          : %ld\n"
           "Bytes Sent            : %ld\n"
           "Acks Received         : %ld\n"
           "Acks Bytes Received   : %ld\n"
           "Dup Acks Received     : %ld\n"
           "Dup Acks Bytes Rcvd   : %ld\n"
           "Timer retrans pkts    : %ld\n"
           "Timer retrans bytes   : %ld\n"
           "Fast Retransmit pkts  : %ld\n"
           "Fast Retransmit bytes : %ld\n",
           s->stat[P2MPC_STAT_PKTS_SENT][i],
           s->stat[P2MPC_STAT_BYTES_SENT][i],
           s->stat[P2MPC_STAT_ACKS_RCVD][i],
           s->stat[P2MPC_STAT_ACKS_BYTES_RCVD][i],
           s->stat[P2MPC_STAT_DUPACKS_RCVD][i],
           s->stat[P2MPC_STAT_DUPACKS_BYTES_RCVD][i],
           s->stat[P2MPC_STAT_TRTRANS_PKTS_SENT][i],
           s->stat[P2MPC_STAT_TRTRANS_BYTES_SENT][i],
           s->stat[P2MPC_STAT_FRTRANS_PKTS_SENT][i],
           s->stat[P2MPC_STAT_FRTRANS_BYTES_SENT][i]);
  }

  compute_time_diff(&(pcb->start_time), &(pcb->end_time), &diff);

  if(pcb->md5[0]) {
    printf("\nmd5 checksum of file : %s\n", pcb->md5);
  }
  printf("\nTime for transfer    : %ld.%ld s\n", diff.tv_sec, diff.tv_usec);
}

void buffer_init(p2mp_pcb *pcb) {
  int i = 0;
  node *node_ptr = NULL, *node_mv = NULL;

  while(i < pcb->N) {

    node_ptr = (node*)malloc(sizeof(node));
    if(node_ptr == NULL) {
      die("BUFFER_INIT : malloc failed!", errno);
    }

    node_ptr->filled = 0;
    node_ptr->next = NULL;
    P2MP_ZERO(node_ptr->acks);

    if(pcb->win.head == NULL) {
      pcb->win.head = node_ptr;
      pcb->win.to_send = node_ptr;
      pcb->win.tail = node_ptr;
    }
    else {
      node_mv = pcb->win.head;
      while(node_mv->next != NULL)
        node_mv = node_mv->next;
      node_mv->next = node_ptr;
      pcb->win.tail = node_ptr;
    }
    ++i;
  }
}

void buffer_delete(p2mp_pcb *pcb) {

  node *node_ptr, *node_next;

  node_ptr = pcb->win.head;
  while(node_ptr) {
    node_next = node_ptr->next;
    free(node_ptr);
    node_ptr = node_next;
  }
}

int main(int argc, char *argv[])
{
  int i, st = 0;
  char *cfg_file = NULL;

  struct sigevent sev;

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
    pcb.mss = atoi(argv[argc-1]);
    if(pcb.mss < 0 || pcb.mss > BUFFER_SIZE) {
      usage();
    }
    pcb.N = atoi(argv[argc-2]);
    strncpy(pcb.filename, argv[argc-3], FILE_NSIZE);

    for(i = 1 ; i < argc-4 && st <= MAX_RECV ; i+=2) {
      pcb.recv[st].sin_family = AF_INET;
      pcb.recv[st].sin_addr.s_addr = inet_addr(argv[i]);
      pcb.recv[st].sin_port = htons(atoi(argv[i+1]));
      ++st;
    }

    pcb.num_recv = st;
  }

  pcb.win.num_empty = pcb.N;

  // Setup socket
  pcb.sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // Setup timer
  sev.sigev_notify = SIGEV_THREAD;
  sev.sigev_notify_function = (void*)timer;
  sev.sigev_notify_attributes = NULL;
  sev.sigev_value.sival_ptr = (void*)&pcb;

  if(timer_create(CLOCK_REALTIME, &sev, &pcb.timerid) == -1) {
    die("MAIN : timer_create failed!", 0);
  }

  buffer_init(&pcb);

  gettimeofday(&pcb.start_time, NULL);

  pthread_create(&(pcb.buf_mgr), NULL, rdt_send, &pcb);
  pthread_create(&(pcb.sender), NULL, sender, &pcb);
  pthread_create(&(pcb.receiver), NULL, receiver, &pcb);

  pthread_join(pcb.buf_mgr, NULL);
  pthread_join(pcb.sender, NULL);
  pthread_join(pcb.receiver, NULL);

  close(pcb.sockfd);

  gettimeofday(&pcb.end_time, NULL);

  buffer_delete(&pcb);

  print_stats(&pcb);

  return 0;
}
