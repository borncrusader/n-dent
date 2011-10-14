#include <stdio.h>

#include "p2mp.h"

void* rdt_send(void *args) {
  p2mp_pcb *pcb;
  int N, avail, mss;

  pcb = (p2mp_pcb*)args;

  N = pcb->N;
  mss = pcb->mss;

  return;
}
