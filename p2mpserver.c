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


int main(int argc, char *argv[])
{
  p2mp_sb serv;
  P2MP_ZERO(serv);
  struct sockaddr_in server,sender;
  socklen_t len;
  char ack_buf[8];
  float drop_pkt;
  int sock_status_recv;
  char buf[BUFFER_SIZE];
  struct node buf_data[atoi(argv[3])];
  int fill_here,i;

  int ret = 0, seq_num = 0, type = 0, flags=0,prev_seq_num=-1,next_there=0, last_seq_num = -1,run_flag=1,count=0;

  char from[INET_ADDRSTRLEN];
  FILE *fp;

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
    die("p2mpserver : socket creation failed!", errno);
  }

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port= (htons(atoi(argv[1])));	

  if(bind(serv.sock_server_recv, (struct sockaddr*)&server, sizeof(server)) == -1) {
    close(serv.sock_server_recv);
    die("p2mpserver : bind failed!", errno);
  }


  fp = fopen(serv.filename, "w");
  if(fp == NULL) {
    die("p2mpserver : file cannot be opened!", errno);
  }

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
      warn("p2mpserver: checksum error!", 0);
      continue;
    }

    drop_pkt=(rand()%1000)/1000.0;

    if(drop_pkt<serv.p)
    {
      printf("Packet loss, sequence number = %d\n", seq_num);
      continue;
    }

    if(flags&FLAG_EOM) {
      last_seq_num = seq_num;
	        printf("Recieved EOM, sequence number = %d\n", seq_num);
  }

    if(seq_num==prev_seq_num+1)
    {
      printf("Received in-sequence packet from %s:%d, sequence number = %d, ",
             from, htons(sender.sin_port), seq_num);
      //printf("writing %d bytes of %d\n", ret-HEADER_SIZE, seq_num);
      fwrite(buf+HEADER_SIZE,ret-HEADER_SIZE,1,fp);
	printf("Writing packet with seq num %d\n",seq_num);
      fflush(fp);
      prev_seq_num=seq_num;
      /*
         check the to_buffer[] struct array repeatedly for any packet that is buffered and has seq_num = cur_seq_num+1;
         if (present) remove that by setting filled=0 
         and write that contents to file
         and set seq_num=seq_num of the packet that was just written
         */

//      while(1)
  //    {
        for(i=0;i<serv.N;(i+1)%serv.N)
        {
          if(buf_data[i].filled==1 && buf_data[i].seqnum==prev_seq_num+1)
          {
            //printf("writing %d bytes of %d\n", buf_data[i].buf_size-HEADER_SIZE, seq_num);
            //printf("writing %d\n", seq_num);
            fwrite(buf_data[i].buf+HEADER_SIZE,buf_data[i].buf_size-HEADER_SIZE,1,fp);
            fflush(fp);
	printf("Writing packet with seq num %d\n",buf_data[i].seqnum);
            buf_data[i].filled=0;
            prev_seq_num=buf_data[i].seqnum;
	    count=0;	
          }
		count++;
	
	if(count==10)
	break;

          //if(buf_data[i].seqnum==prev_seq_num+1)
            //next_there=1;
        }

        //if(next_there!=1)
        //  break;
      //}

      pack_data(prev_seq_num, MSG_TYPE_ACK, 0, ack_buf, 8);//CREATE THE ACK
      printf("Sending ack for sequence number = %d\n", prev_seq_num);
      sendto(serv.sock_server_recv, ack_buf, 8, 0, (struct sockaddr*)&sender, sizeof(sender));//SEND THE ACK

      if(last_seq_num == seq_num) {
		        printf("Written the last packet, sequence number = %d\n", seq_num);
			printf("TRANSFER COMPLETE :QUITTING \n");
        break;
      }
    }
    else{
      printf("Received out-of-sequence packet from %s:%d, sequence number = %d, ",
             from, htons(sender.sin_port), seq_num);
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
        {fill_here=i;
          break;}
      }

      if(fill_here==-1)
      {
        printf("Oops! The world is going to end! Buffer full cannot save packet. Dropping it!\n\n");
      }

else { 
     memcpy(buf_data[fill_here].buf,buf,ret);
      buf_data[fill_here].filled=1;
      buf_data[fill_here].seqnum=seq_num;
      buf_data[fill_here].buf_size=ret;

 
        pack_data(prev_seq_num, MSG_TYPE_ACK, 0, ack_buf, 8);//CREATE THE ACK
        printf("Sending ack for sequence number = %d\n", prev_seq_num);
        sendto(serv.sock_server_recv, ack_buf, 8, 0, (struct sockaddr*)&sender, sizeof(sender));//SEND THE prev ACK
      }        
    }
  }

  close(serv.sock_server_recv);

  return 0;
}
