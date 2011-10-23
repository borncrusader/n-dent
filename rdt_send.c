#include "p2mp.h"
#include "p2mpclient.h"

void buffer_init(p2mp_pcb *pcb) {
  int i = 0, N = 0;
  node *node_ptr = NULL, *node_mv = NULL;

  N = pcb->N;

  while(i < N) {

    node_ptr = (node*)malloc(sizeof(node));
    if(node_ptr == NULL) {
      die("buffer_init: malloc failed! : ", errno);
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

void* rdt_send(void *args) {
  p2mp_pcb *pcb;
  node *node_ptr;
  int c;
  int buf_size, flags;
  int looper = 1;
  char buf[BUFFER_SIZE];
  FILE *fp;

  pcb = (p2mp_pcb*)args;

  buffer_init(pcb);

  fp = fopen(pcb->filename, "rb");
  if(fp == NULL) {
    die("rdt_send : file cannot be opened! : ", errno);
  }

  while(looper) {
    flags = 0;

    memset(buf, 0, sizeof(buf));

    buf_size = fread(buf, 1, pcb->mss, fp);
    if(ferror(fp)) {
      fclose(fp);
      die("rdt_send: file i/o error: ", errno);
    }

    // peek and unpeek last character
    c = fgetc(fp);
    ungetc(c,fp);

    if (c == -1) {
      printf("rdt_send : EOM reached!\n");
      flags = FLAG_EOM;
      looper = 0;
    }

    printf("rdt_send : %d read from file..\n", buf_size);
 
    pthread_mutex_lock(&(pcb->win.win_lck));

    printf("rdt_send : Num of empty nodes in window : %d\n", pcb->win.num_empty);
 
    if(pcb->win.num_empty > 0) {

      node_ptr = pcb->win.head;
      while(node_ptr && node_ptr->filled == 1) {
        node_ptr = node_ptr->next;
      }

      memcpy(node_ptr->buf+HEADER_SIZE, buf, buf_size);

      node_ptr->filled = 1;
      node_ptr->buf_size = buf_size + HEADER_SIZE; // including the header!
      node_ptr->flags = flags;
 
      --(pcb->win.num_empty);

      pcb->win.data_available = 1;
    }
    else {
      if(fseek(fp, -buf_size, SEEK_CUR) == -1) {
        die("rdt_send : fseek failed! : ", errno);
      }
      printf("rdt_send : fseek'ing back\n");
    }

    pthread_mutex_unlock(&(pcb->win.win_lck));
    sleep(BUF_TIMEOUT);
  }

  fclose(fp);

  return;
}
