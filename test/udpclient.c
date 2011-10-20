#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include "common.h"

int main(int argc, char *argv[])
{
  int sock, ret, port;
  struct sockaddr_in dest_addr;
  int addr_size;
  unsigned char buf[BUFFER_SIZE];

  port = atoi(argv[1]);

  if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    perror("client: socket");
    return 1;
  }

  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  dest_addr.sin_addr.s_addr = inet_addr(argv[2]);

  //snprintf(buf, BUFFER_SIZE, "Hello World!");

  addr_size = sizeof(dest_addr);
  {
    FILE *fp;
    int fret, seq_num=-1;
    int c, flags;

    fp = fopen(argv[3], "r");

    if(fp != NULL) {
      // send the filename first
      int fname_len = strlen(argv[3]);

      strncpy(buf+HEADER_SIZE, argv[3], fname_len);
      pack_data(seq_num, MSG_TYPE_DATA, FLAG_FNAME, buf, fname_len+HEADER_SIZE);

      if((ret = sendto(sock, buf, fname_len+HEADER_SIZE,0,(struct sockaddr*)&dest_addr,addr_size)) == -1) {
        close(sock);
        perror("client: sendto");
        return 1;
      }
      
      seq_num++;

      // blast away the data, i say!
      while((fret = fread(buf+HEADER_SIZE, 1, DATA_SIZE, fp)) != 0) {
        flags = 0;
        printf("read %d bytes!\n", fret);

        // peek and unpeek last character
        c = fgetc(fp);
        ungetc(c,fp);

        if (c == -1) {
          flags = FLAG_EOM;
        }

        pack_data(seq_num, MSG_TYPE_DATA, flags, buf, fret+HEADER_SIZE);
        if((ret = sendto(sock, buf, fret+HEADER_SIZE,0,(struct sockaddr*)&dest_addr,addr_size)) == -1) {
          close(sock);
          perror("client: sendto");
          return 1;
        }

        seq_num++;
      }
      fclose(fp);
    }
  }

  return 0;
}
