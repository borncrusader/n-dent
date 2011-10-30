#include "p2mp.h"
#include "p2mpclient.h"

void* rdt_send(void *args) {
  p2mp_pcb *pcb;
  node *node_ptr;
  int c;
  int buf_size, flags;
  int looper = 1;
  char buf[BUFFER_SIZE];
  FILE *fp;

  pcb = (p2mp_pcb*)args;

  fp = fopen(pcb->filename, "rb");
  if(fp == NULL) {
    die("RDT_SEND : File cannot be opened!", errno);
  }

  while(looper) {
    flags = 0;

    memset(buf, 0, sizeof(buf));

    buf_size = fread(buf, 1, pcb->mss, fp);
    if(ferror(fp)) {
      fclose(fp);
      die("RDT_SEND : File i/o error", errno);
    }

    // peek and unpeek last character
    c = fgetc(fp);
    ungetc(c,fp);

    if (c == -1) {
      //printf("RDT_SEND : EOM reached buf_size : %d!\n", buf_size);
      flags = FLAG_EOM;
    }

    //printf("RDT_SEND : %d read from file..\n", buf_size);
 
    pthread_mutex_lock(&(pcb->win.win_lck));

    if(pcb->win.num_empty > 0) {

      node_ptr = pcb->win.head;
      while(node_ptr && node_ptr->filled == 1) {
        node_ptr = node_ptr->next;
      }

      //printf("rdt_send : num_empty %d node_ptr %p head %p\n", pcb->win.num_empty, node_ptr, pcb->win.head);

      memcpy(node_ptr->buf+HEADER_SIZE, buf, buf_size);

      node_ptr->filled = 1;
      node_ptr->buf_size = buf_size + HEADER_SIZE; // including the header!
      node_ptr->flags = flags;
 
      --(pcb->win.num_empty);

      pcb->win.data_available = 1;

      if(flags & FLAG_EOM) {
        printf("RDT_SEND : EOM Reached, buf_size : %d\n", buf_size);
        looper = 0;
      }

      printf("RDT_SEND : Filling buffer... empty = %d buf_size = %d\n", pcb->win.num_empty, buf_size);

      P2MPC_STAT_INCREMENT(&(pcb->stat), P2MPC_STAT_RDT_SEND_PKTS_READ, 0);
      P2MPC_STAT_UPDATE(&(pcb->stat), P2MPC_STAT_RDT_SEND_BYTES_READ, 0, buf_size);
    }
    else {
      if(fseek(fp, -buf_size, SEEK_CUR) == -1) {
        die("RDT_SEND : fseek failed!", errno);
      }
      //printf("RDT_SEND : fseek'ing back\n");
    }

    //printf("RDT_SEND : Num of empty nodes in window : %d\n", pcb->win.num_empty);

    pthread_mutex_unlock(&(pcb->win.win_lck));
    //sleep(BUF_TIMEOUT);
  }

  fclose(fp);

  return;
}
