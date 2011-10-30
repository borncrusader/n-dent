#ifndef __P2MP_H
#define __P2MP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <sys/types.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
 * ------------------------
 * |   32 bits seq num    |
 * ------------------------
 * | checksum |type |flags|
 * ------------------------
 * |                      |
 * |         data         |
 * |                      |
 *
 * flags = | | | | | | | |EOM|
 *
 * EOM - End of message. Set as 1 to indicate the last packet of the file.
 */

#define FILE_NSIZE  100               // File name size
#define MAX_RECV    10                // Maximum number of receivers

#define BUF_TIMEOUT 2                 // buffer aggregator timeout
#define ACK_SEC 0                     // timeout value for ack in secs, default 0 sec
#define ACK_NSEC 30000000             // timeout value for ack in nanosecs, default 30msec 

#define BUFFER_SIZE 1500              // Maximum segment size of a packet
#define HEADER_SIZE 8                 // size of the protocol header

#define MSG_TYPE_DATA 0x55            // type of the message - data
#define MSG_TYPE_ACK 0xaa             // type of the message - ack

#define FLAG_EOM 0x01                 // denoting the end of message

#define P2MP_ZERO(p) \
  memset(&(p), 0, sizeof(p))

#define P2MP_ZERO_ADDR(p, q) \
  memset((p), 0, sizeof(q))

#define DATA_SIZE (BUFFER_SIZE-HEADER_SIZE)

void die(const char*, int);
void warn(const char*, int);

void compute_checksum(uint16_t*, unsigned char*, int);

int pack_data(int, int, int, unsigned char*, int);
int unpack_data(int*, int*, int*, unsigned char*, int);

#endif
