#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef __COMMON_H__
#define __COMMON_H__

#define MSG_TYPE_DATA 0x55
#define MSG_TYPE_ACK 0xaa

#define FLAG_EOM 0x01
#define FLAG_FNAME 0x02

#define BUFFER_SIZE 1400
#define HEADER_SIZE 8
#define DATA_SIZE (BUFFER_SIZE-HEADER_SIZE)

typedef unsigned char bool;
#define TRUE 1
#define FALSE 0

int pack_data(int, int, int, unsigned char*, int);
int unpack_data(int*, int*, int*, unsigned char*, int);

void compute_checksum(uint16_t*, unsigned char*, int);

/*struct buffer_elem;

typedef struct
{
  uint32_t seq_start; // seq num start
  int win_size;  // total num of buffers
  int buf_len;   // smaller than win_size
  struct buffer_elem *hbuf; // head of the buffer
}buffer_t;

typedef struct buffer_elem
{
  bool acked;
  uint32_t seq_num;
  int buf_size;
  unsigned char *buf;
  struct buffer_elem *next;
}buffer_elem_t;

int buffer_init(buffer_t*, int, int);
int buffer_free(buffer_t*);
int buffer_insert(buffer_t*, uint32_t, unsigned char*);
int buffer_remove();

int buffer_elem_init(buffer_elem_t*);
*/

#endif
