#include "p2mp.h"
#include "p2mpclient.h"

void buffer_init(p2mp_pcb *pcb) {
  int i = 0, N = 0;
  node *win_ptr = NULL, *win_mv = NULL;

  N = pcb->N;

  while(i < N) {

    win_ptr = (node*)malloc(sizeof(node));
    if(win_ptr == NULL) {
      die("buffer_init: malloc failed! : ", errno);
    }

    win_ptr->filled = 0;
    win_ptr->next = NULL;
    P2MP_ZERO(win_ptr->acks);

    if(pcb->win.head == NULL) {
      pcb->win.head = win_ptr;
      pcb->win.tosend = win_ptr;
      pcb->win.left = win_ptr;
      pcb->win.right = win_ptr;
    }
    else {
      win_mv = pcb->win.head;
      while(win_mv->next != NULL)
        win_mv = win_mv->next;
      win_mv->next = win_ptr;
      pcb->win.right = win_ptr;
    }
    ++i;
  }
}

void* rdt_send(void *args) {
  p2mp_pcb *pcb;
  node *win_ptr;
  int i, N, num_empty, mss;
  unsigned char looper = 1;
  char buf[BUFFER_SIZE];
  FILE *fp;

  pcb = (p2mp_pcb*)args;
  N = pcb->N;
  mss = pcb->mss;

  buffer_init(pcb);

  fp = fopen(pcb->filename, "rb");

  while(looper) {

    P2MP_ZERO(buf);

    if(!fread(buf, 1, pcb->mss, fp)) {
      if(feof(fp)) {
        pcb->eof = 1;
        looper = 0;
      }
      else if(ferror(fp)) {
        fclose(fp);
        die("rdt_send: file i/o error: ", errno);
      }
    }

    printf("rdt_send: MSS read from file..\n");
 
    pthread_mutex_lock(&(pcb->win.win_lck));
 
    num_empty = pcb->win.num_empty;
 
    if(num_empty < N) {

      win_ptr = pcb->win.head;
      while(win_ptr && win_ptr->filled == 1) {
        win_ptr = win_ptr->next;
      }

      strncpy(win_ptr->buf, buf, BUFFER_SIZE);

      win_ptr->filled = 1;
 
      --(pcb->win.num_empty);
      ++(pcb->win.num_avail);

      //send thread should block on a condition variable

      printf("rdt_send: pthread_cond_signal..\n");
      pthread_cond_signal(&(pcb->win.win_cnd));
    }
    else {
      fseek(fp, -(pcb->mss), SEEK_CUR);
    }

    pthread_mutex_unlock(&(pcb->win.win_lck));
    sleep(BUF_TIMEOUT);
  }

  fclose(fp);

  return;
}
