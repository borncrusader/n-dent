#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include "common.h"

int main(int argc, char *argv[])
{
  int sock, ret, port, bytes_rcvd;
  struct sockaddr_in src_addr, dest_addr;
  int addr_size;
  unsigned char *buf;
  buffer_t buffer;

  buffer_init(&buffer, 0, 5);

  port = atoi(argv[1]);

  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    perror("server: socket");
    return 1;
  }

  src_addr.sin_family = AF_INET;
  src_addr.sin_port = htons(port);
  src_addr.sin_addr.s_addr = INADDR_ANY;

  if(bind(sock, (struct sockaddr*)&src_addr, sizeof(src_addr)) == -1) {
    close(sock);
    perror("server: bind");
    return 1;
  }

  addr_size = sizeof(dest_addr);
  {
    FILE *fp;
    int seq_num, type, flags;
    char *fname;

    buf = (char*)malloc(sizeof(char)*BUFFER_SIZE);
    bytes_rcvd = recvfrom(sock, buf, BUFFER_SIZE,0,(struct sockaddr*)&dest_addr,&addr_size);
    buf[bytes_rcvd] = '\0';
    ret = unpack_data(&seq_num, &type, &flags, buf, bytes_rcvd);
    if(ret == 0) {
      printf("server : unpack failed!\n");
    } else if(ret == -1) {
      printf("server : checksum failed!\n");
    } else {
      fname = basename(buf+HEADER_SIZE);
      fp = fopen(fname, "w");
    }

    buffer_insert(&buffer, seq_num, buf);

    while(1) {
      buf = (char*)malloc(sizeof(char)*BUFFER_SIZE);
      bytes_rcvd = recvfrom(sock, buf, BUFFER_SIZE,0,(struct sockaddr*)&dest_addr,&addr_size);

      if(bytes_rcvd == -1) {
        perror("server: recvfrom");
        break;
      } else if(bytes_rcvd == 0) {
        break;
      }

      ret = unpack_data(&seq_num, &type, &flags, buf, bytes_rcvd);
      if(ret == 0) {
        printf("server : unpack failed!\n");
      } else if(ret == -1) {
        printf("server : checksum failed!\n");
      } else {
        //fwrite(buf+HEADER_SIZE, 1, bytes_rcvd-HEADER_SIZE, fp);
        fwrite(buf+HEADER_SIZE, bytes_rcvd-HEADER_SIZE, 1, fp);
      }

      buffer_insert(&buffer, seq_num, buf);
      if(flags&FLAG_EOM) {
        break;
      }
    }
    fclose(fp);
  }

  close(sock);

  buffer_free(&buffer);

  return 0;
}
