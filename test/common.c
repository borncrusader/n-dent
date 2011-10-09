#include "common.h"

int pack_data(int seq_num, int type, int flags, unsigned char *buf, int size)
{
  uint16_t csum;

  if(!buf) {
    return 0;
  }

  buf[0] = (seq_num & 0xff000000)>>24;
  buf[1] = (seq_num & 0x00ff0000)>>16;
  buf[2] = (seq_num & 0x0000ff00)>>8;
  buf[3] = (seq_num & 0x000000ff);
  buf[4] = 0; // zero it for checksum computation
  buf[5] = 0; // zero it for checksum computation
  buf[6] = type;
  buf[7] = flags;

  compute_checksum(&csum, buf, size);

  printf("computed : %x\n", csum);

  buf[4] = (csum & 0xff00)>>8;
  buf[5] = (csum & 0x00ff);

  //printf("header\n|%02x%02x %02x %02x|\n|%02x%02x|%02x|%02x|\n",
  //       buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);

  return 1;
}

int unpack_data(int *seq_num, int *type, int *flags, unsigned char *buf, int size)
{
  uint16_t msg_csum=0, comp_csum=0;

  if(!buf) {
    return 0;
  }

  *seq_num  = 0;
  *seq_num |= buf[0]<<24;
  *seq_num |= buf[1]<<16;
  *seq_num |= buf[2]<<8;
  *seq_num |= buf[3];

  *type     = buf[6];
  *flags    = buf[7];

  msg_csum |= buf[4]<<8;
  msg_csum |= buf[5];

  compute_checksum(&comp_csum, buf, size);

  printf("computed : %x, from_data : %x\n", comp_csum, msg_csum);

  //printf("header\n|%02x%02x %02x %02x|\n|%02x%02x|%02x|%02x|\n",
  //       buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7]);

  if(comp_csum) {
    // computed checksum should be 0. As we do another ~ in the compute_checksum function
    return -1;
  }

  return 1;
}

void compute_checksum(uint16_t *csum, unsigned char *buf, int size)
{
  int i;
  int sum = 0;

  for(i=0;i<size;i+=2) {
    if(i+1<size) {
      sum += ((buf[i]<<8)|(buf[i+1]));
    } else {
      sum += buf[i]<<8;
    }
    if(sum&0x10000) {
      // overlap carry
      sum = (sum & 0xffff) + 1;
    }
    printf("%x\t", sum);
  }

  printf("\n");

  *csum = sum;
  *csum = ~(*csum);

  return;
}

int buffer_elem_init(buffer_elem_t *buf)
{
  if(buf) {
    buf->seq_num = 0;
    buf->buf_size = 0;
    buf->acked = TRUE; // initially true so that it is free to be replaced!
    buf->buf = NULL;
    buf->next = NULL;

    return 1;
  }

  return 0;
}

int buffer_init(buffer_t *buffer, int seq_start, int win_size)
{
  buffer_elem_t *p = NULL, *prev = NULL;
  int i;

  if(buffer) {
    buffer->seq_start = seq_start;
    buffer->win_size = win_size;
    buffer->buf_len = 0;
    buffer->hbuf = NULL;

    // create win_size buffer elements
    for(i=0;i<buffer->win_size;i++) {
      p = (buffer_elem_t*)malloc(sizeof(buffer_elem_t));
      if(buffer_elem_init(p)) {
        if(i==0) {
          buffer->hbuf = p;
        } else {
          prev->next = p;
        }
        prev = p;
      } else {
        printf("buffer element cannot be initialised!\n");
        free(p);
      }
    }

    return 1;
  }

  return 0;
}

int buffer_free(buffer_t *buffer)
{
  buffer_elem_t *p = NULL, *q = NULL;

  if(buffer) {
    p = buffer->hbuf;
    buffer->hbuf = NULL;

    while(p) {
      q = p->next;
      p->next = NULL;
      if(p->buf) {
        free(p->buf);
      }
      free(p);
      p = q;
    }
  }

  return 0;
}

int buffer_insert(buffer_t *buffer, uint32_t seq_num, unsigned char *buf)
{
  buffer_elem_t *p;

  if(buffer) {
    p = buffer->hbuf;

    while(p) {
      if(p->acked == TRUE) {
        p->seq_num = seq_num;
        p->buf = buf;
        p->acked = FALSE;
        break;
      }
      p = p->next;
    }

    return 1;
  }

  return 0;
}
