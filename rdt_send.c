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
      pcb->win.tosend = node_ptr;
      pcb->win.left = node_ptr;
      pcb->win.right = node_ptr;
    }
    else {
      node_mv = pcb->win.head;
      while(node_mv->next != NULL)
        node_mv = node_mv->next;
      node_mv->next = node_ptr;
      pcb->win.right = node_ptr;
    }
    ++i;
  }
}

void* rdt_send(void *args) {
  p2mp_pcb *pcb;
  node *node_ptr;
  int c, N, num_empty, mss;
  int buf_size, flags;
  unsigned char looper = 1;
  char buf[BUFFER_SIZE];
  FILE *fp;

  pcb = (p2mp_pcb*)args;
  N = pcb->N;
  mss = pcb->mss;

  buffer_init(pcb);

  fp = fopen(pcb->filename, "rb");
  if(fp == NULL) {
    die("rdt_send : file cannot be opened! : ", errno);
  }

  while(looper) {
    flags = 0;

    memcpy(buf, 0, sizeof(buf));

    buf_size = fread(buf, 1, pcb->mss, fp);
    if(ferror(fp)) {
      fclose(fp);
      die("rdt_send: file i/o error: ", errno);
    }

    // peek and unpeek last character
    c = fgetc(fp);
    ungetc(c,fp);

    if (c == -1) {
      flags = FLAG_EOM;
      looper = 0;
    }

    printf("rdt_send: %d read from file..\n", buf_size);
 
    pthread_mutex_lock(&(pcb->win.win_lck));
 
    num_empty = pcb->win.num_empty;
 
    if(num_empty < N) {

      node_ptr = pcb->win.head;
      while(node_ptr && node_ptr->filled == 1) {
        node_ptr = node_ptr->next;
      }

      strncpy(node_ptr->buf, buf, BUFFER_SIZE);

      node_ptr->filled = 1;
      node_ptr->buf_size = buf_size;
      node_ptr->flags = flags;
 
      --(pcb->win.num_empty);
      ++(pcb->win.num_avail);

      pcb->win.data_available = 1;
    }
    else {
      fseek(fp, -buf_size, SEEK_CUR);
    }

    pthread_mutex_unlock(&(pcb->win.win_lck));
    sleep(BUF_TIMEOUT);
  }

  fclose(fp);

  return;
}
