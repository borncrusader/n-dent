#include "p2mp.h"
#include "p2mpserver.h"

void usage() {
  printf("p2mpserver port# file-name N p\n");
  printf("           port# - port number to which the server is listening\n");
  printf("           file-name - file where the data will be written\n");
  printf("           N - window size\n");
  printf("           p - probability of packet loss, ranges between 0 and 1\n");
  exit(1);
}

void print_stats(p2mp_sb *s)
{
  char md5[MD5_LEN+1];
  int c;

  printf("\nStatistics\n");
  printf("Acks pkts sent        : %ld\n"
         "Acks bytes sent       : %ld\n"
         "Dup Acks pkts sent    : %ld\n"
         "Dup Acks bytes sent   : %ld\n\n",
         s->stat[P2MPS_STAT_ACKS_SENT],
         s->stat[P2MPS_STAT_ACKS_BYTES_SENT],
         s->stat[P2MPS_STAT_DUPACKS_SENT],
         s->stat[P2MPS_STAT_DUPACKS_BYTES_SENT]);
  printf("Pkts received in-seq  : %ld\n"
         "Bytes received in-seq : %ld\n"
         "Pkts rcvd out-of-seq  : %ld\n"
         "Bytes rcvd out-of-seq : %ld\n"
         "Pkts rcvd redundant   : %ld\n"
         "Bytes rcvd redundant  : %ld\n\n",
         s->stat[P2MPS_STAT_PKTS_IS_RCVD],
         s->stat[P2MPS_STAT_BYTES_IS_RCVD],
         s->stat[P2MPS_STAT_PKTS_OS_RCVD],
         s->stat[P2MPS_STAT_BYTES_OS_RCVD],
         s->stat[P2MPS_STAT_PKTS_NR_RCVD],
         s->stat[P2MPS_STAT_BYTES_NR_RCVD]);
  printf("Pkts corrupt          : %ld\n"
         "Bytes corrupt         : %ld\n\n",
         s->stat[P2MPS_STAT_PKTS_CORRUPT],
         s->stat[P2MPS_STAT_BYTES_CORRUPT]);
  printf("Pkts dropped          : %ld\n"
         "Bytes dropped         : %ld\n\n",
         s->stat[P2MPS_STAT_PKTS_DROP],
         s->stat[P2MPS_STAT_BYTES_DROP]);

  printf("File saved with filename %s\n", s->filename);
  if(s->md5[0]) {
    c = compute_md5(s->filename, md5);

    printf("md5 checksum from packet   : %s\n", s->md5);
    if(c == 1) {
      printf("md5 checksum of saved file : %s\n", md5);
    }
  }
}

int main(int argc, char *argv[])
{
  p2mp_sb serv;
  P2MP_ZERO(serv);
  struct sockaddr_in server,sender;
  socklen_t len;
  char ack_buf[HEADER_SIZE];
  float drop_pkt;
  int sock_status_recv;
  char buf[BUFFER_SIZE];
  node buf_data[atoi(argv[3])];
  int fill_here,i;

  int ret = 0, seq_num = 0, type = 0, flags=0,prev_seq_num=-1,next_there=0, last_seq_num = -1,run_flag=1,count=0,prev_ackd=-1;

  char from[INET_ADDRSTRLEN];
  FILE *fp = NULL;

  P2MP_ZERO(serv);

  srand(time(0));

  if(argc!=5) {
    usage();
  }
  else if(atof(argv[4]) < 0 || atof(argv[4]) > 1) {
    usage();
  }

  serv.p = atof(argv[4]);
  serv.N = atoi(argv[3]);
  strncpy(serv.filename, argv[2],FILE_NSIZE);

  if((serv.sock_server_recv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    die("p2mpserver : Socket creation failed!", errno);
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port= (htons(atoi(argv[1])));	

  if(bind(serv.sock_server_recv, (struct sockaddr*)&server, sizeof(server)) == -1) {
    close(serv.sock_server_recv);
    die("p2mpserver : Bind failed!", errno);
  }


  /*
  fp = fopen(serv.filename, "w");
  if(fp == NULL) {
    die("p2mpserver : File cannot be opened!", errno);
  }
  */

  len=sizeof(sender);

  while(run_flag)
  {
    flags = 0;
    ret = recvfrom(serv.sock_server_recv, buf, BUFFER_SIZE, 0, (struct sockaddr*)&sender, &len);
    if(ret == 0) { 
      warn("p2mpserver: recvfrom() client closed connection!", errno);
      continue;
    } else if(ret == -1) {
      warn("p2mpserver: recvfrom() error!", errno);
      continue;
    }

    inet_ntop(AF_INET, &(sender.sin_addr), from, INET_ADDRSTRLEN);
    from[INET_ADDRSTRLEN-1] = '\0';

    if(unpack_data(&seq_num, &type, &flags, buf, ret) == -1) {
      warn("p2mpserver: Checksum error!", 0);
      P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_PKTS_CORRUPT);
      P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_BYTES_CORRUPT, ret);
      continue;
    }

    drop_pkt=(rand()%1000)/1000.0;

    if(drop_pkt<serv.p)
    {
      printf("Packet loss, sequence number = %d\n", seq_num);
      P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_PKTS_DROP);
      P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_BYTES_DROP, ret);
      continue;
    }

    if(flags&FLAG_EOM) {
      last_seq_num = seq_num;
      printf("Received EOM, sequence number = %d\n", seq_num);
    }

    if(seq_num==prev_seq_num+1)
    {
      printf("Received in-sequence packet from %s:%d, sequence number = %d, ",
          from, htons(sender.sin_port), seq_num);

      P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_PKTS_IS_RCVD);
      P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_BYTES_IS_RCVD, ret);

      if(flags&FLAG_FNAME) {
        // first packet arriving with the filename/md5, open the file pointer
        int offset = HEADER_SIZE;
        buf[ret] = '\0';

        if(flags&FLAG_MD5) {
          strncpy(serv.md5, buf+HEADER_SIZE, MD5_LEN);
          serv.md5[MD5_LEN] = '\0';
          offset = HEADER_SIZE+MD5_LEN;
        } else {
          serv.md5[0] = '\0';
        }

        if(access(buf+offset, R_OK) == -1) {
          // file doesn't exist
          warn("Filename in msg doesn't exists, using it", 0);
          strncpy(serv.filename, buf+offset, FILE_NSIZE);
        }

        fp = fopen(serv.filename, "w");
        if(fp == NULL) {
          die("p2mpserver : file cannot be opened!", errno);
        }
      } else {
        //printf("writing %d bytes of %d\n", ret-HEADER_SIZE, seq_num);
        if(fp == NULL) {
          die("dying", 0);
        }
        fwrite(buf+HEADER_SIZE,ret-HEADER_SIZE,1,fp);
        printf("Writing packet with seq num %d\n",seq_num);
        //fflush(fp);
      }
      prev_seq_num=seq_num;
      /*
         check the to_buffer[] struct array repeatedly for any packet that is buffered and has seq_num = cur_seq_num+1;
         if (present) remove that by setting filled=0 
         and write that contents to file
         and set seq_num=seq_num of the packet that was just written
         */

      //      while(1)
      //    {
      count=0;
      for(i=0;i<serv.N;i=((i+1)%serv.N))
      {
        if(buf_data[i].filled==1 && buf_data[i].seqnum==prev_seq_num+1)
        {
          //printf("writing %d bytes of %d\n", buf_data[i].buf_size-HEADER_SIZE, seq_num);
          //printf("writing %d\n", seq_num);
          fwrite(buf_data[i].buf+HEADER_SIZE,buf_data[i].buf_size-HEADER_SIZE,1,fp);
          //fflush(fp);
          printf("Writing packet with sequence number %d\n ",buf_data[i].seqnum);
          buf_data[i].filled=0;
          prev_seq_num=buf_data[i].seqnum;
          count=0;	
        }
        count++;

        if(count==serv.N)
          break;

        //if(buf_data[i].seqnum==prev_seq_num+1)
        //next_there=1;
      }

      //if(next_there!=1)
      //  break;
      //}

      pack_data(prev_seq_num, MSG_TYPE_ACK, 0, ack_buf, HEADER_SIZE);//CREATE THE ACK
      printf("\033[01;37mSending ack for sequence number = %d\n", prev_seq_num);
	printf("%c[%dm", 0x1B, 0);
      sendto(serv.sock_server_recv, ack_buf, HEADER_SIZE, 0, (struct sockaddr*)&sender, sizeof(sender));//SEND THE ACK

      P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_ACKS_SENT);
      P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_ACKS_BYTES_SENT, HEADER_SIZE);

      if(last_seq_num == prev_seq_num) {
        //printf("Written the last packet, sequence number = %d\n", seq_num);
        system("clear");
        printf("TRANSFER COMPLETE : QUITTING \n");
        break;
      }
    }

    else{
      printf("Received out-of-sequence packet from %s:%d, sequence number = %d, ",
          from, htons(sender.sin_port), seq_num);

      P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_PKTS_OS_RCVD);
      P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_BYTES_OS_RCVD, ret);

      /* Have to bufer the packet if there is space available in the buffer
         let to_buffer[] be the struct array
         find the slot where filled=0	
         push the packet into that slot and set filled=1 
         come out of loop
         if there is no slot then it means window buffer is full. DROP PACKET
         ack(last seq_num); 
         */
      fill_here=-1;
      for(i=0;i<serv.N;i++)
      {
        if(buf_data[i].filled==0)
        {
          fill_here=i;
          break;
        }

        if(buf_data[i].seqnum==seq_num)
        {
          fill_here=-2;
          break;
        }
      }

      if(fill_here==-1)
      {
        printf("\033[01;31mBuffer full cannot save packet. Dropping it!\n\n");
	printf("%c[%dm", 0x1B, 0);
      }
      else if(fill_here==-2)
      {
        printf("\n\033[01;33mAlready Exists, %d will not be buffered\n",seq_num);
        printf("%c[%dm", 0x1B, 0);

        P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_PKTS_NR_RCVD);
        P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_BYTES_NR_RCVD, ret);
      }
      else if(prev_seq_num<seq_num) { 
        memcpy(buf_data[fill_here].buf,buf,ret);
        buf_data[fill_here].filled=1;
        buf_data[fill_here].seqnum=seq_num;
        buf_data[fill_here].buf_size=ret;
        //printf("%c[%d;%d;%dm", 0x1B, 5,31,0);        	
        printf("\n\033[22;31m %d was buffered in pos %d",seq_num,fill_here);
        printf("%c[%dm", 0x1B, 0);
        printf("\n");
      }

 
      pack_data(prev_seq_num, MSG_TYPE_ACK, 0, ack_buf, HEADER_SIZE);//CREATE THE ACK

      printf("\033[01;37mSending ack for sequence number = %d\n", prev_seq_num);
	printf("%c[%dm", 0x1B, 0);

      sendto(serv.sock_server_recv, ack_buf, HEADER_SIZE, 0, (struct sockaddr*)&sender, sizeof(sender));//SEND THE prev ACK

      P2MPS_STAT_INCREMENT(&serv, P2MPS_STAT_DUPACKS_SENT);
      P2MPS_STAT_UPDATE(&serv, P2MPS_STAT_DUPACKS_BYTES_SENT, HEADER_SIZE);
    }
  }

  close(serv.sock_server_recv);
  fclose(fp);

  print_stats(&serv);

  return 0;
}
