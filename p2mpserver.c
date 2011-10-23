#include "p2mp.h"
#include "p2mpserver.h"

void usage() {
	printf("Please specify all the arguments\n");
        printf("p2mpserver port# file-name N p\n");
        printf("port# - Port number to which the server is listening\n");
        printf("file-name - File where the data will be written\n");
        printf("N - Window size\n");
        printf("p - Probability of packet loss, ranges between 0 and 1\n");
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
	
	int ret = 0, seq_num = 0, type = 0, run_flag = 1,flags=0,prev_seq_num=-1,next_there=0;

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
		die("Server Socket creation failed",errno);
		return errno;
	}
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port= (htons(atoi(argv[1])));	

	if(bind(serv.sock_server_recv, (struct sockaddr*)&server, sizeof(server)) == -1) {
		close(serv.sock_server_recv);
		die("Server bind failed",errno);
		return errno;
	}


fp = fopen(serv.filename, "w");
  if(fp == NULL) {
    die("SERVER : file cannot be opened! : ", errno);
  }

len=sizeof(sender);

while(run_flag)
{
printf("WAITING FOR DATA: \n");
   ret = recvfrom(serv.sock_server_recv, buf, BUFFER_SIZE, 0, (struct sockaddr*)&sender, &len);

    inet_ntop(AF_INET, &sender, from, INET_ADDRSTRLEN);
    from[INET_ADDRSTRLEN-1] = '\0';

    if(ret == 0) { 
      warn("receiver: received a 0 return value : ", errno);
      
      continue;
    } else if(ret == -1) {
      warn("receiver: recvfrom() error : ", errno);
      continue;
    }


    //printf("receiver: Received packet from %s\n", from);

    if(unpack_data(&seq_num, &type, &flags, buf, ret) == -1) {
      warn("receiver: Checksum error", 0);
      continue;
    }

	printf("Seq number got is %d from %s \n",seq_num,from);

        drop_pkt=(rand()%1000)/1000.0;

        printf("%f is the DROP rate\n",drop_pkt);

        if(seq_num==prev_seq_num+1&& drop_pkt>serv.p)
        {

        printf("TRYING TO WRITE TO FILE \n");
        fwrite(buf+HEADER_SIZE,ret-HEADER_SIZE,1,fp);
        fflush(fp);
        prev_seq_num=seq_num;
               
        /*
        check the to_buffer[] struct array repeatedly for any packet that is buffered and has seq_num = cur_seq_num+1;
        if (present) remove that by setting filled=0 
        and write that contents to file
        and set seq_num=seq_num of the packet that was just written
        */

        while(1)
        {
                for(i=0;i<serv.N;(i++)%serv.N)
                {
                        if(buf_data[i].filled==1 && buf_data[i].seqnum==prev_seq_num+1)
                        {
                        fwrite(buf_data[i].buf+HEADER_SIZE,sizeof(buf_data[i].buf)-HEADER_SIZE,1,fp);
                        fflush(fp);
                        buf_data[i].filled=0;
                        prev_seq_num=buf_data[i].seqnum;
                        }
                                                      
                        if(buf_data[i].seqnum==prev_seq_num+1)
                               next_there=1;
                }
        
                if(next_there!=1)
                break;
         }


        



  
        pack_data(prev_seq_num, MSG_TYPE_ACK, 0, ack_buf, 8);//CREATE THE ACK
        printf("Sending ACK FOR %d\n",seq_num);
        sendto(serv.sock_server_recv, ack_buf, 8, 0, (struct sockaddr*)&sender, sizeof(sender));//SEND THE ACK
//        prev_seq_num=seq_num;
        }

        else if(drop_pkt<serv.p)
        {
        printf("DROPPED PACKET:::: \n");
        }

        else{

        printf("Out of ORDER PACKET : \n ");
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
                fill_here=i;

                break;
                }

           if(fill_here==-1)
            {
             printf("OOPs Buffer full cannot save packet. Dropping it ");
            }
        
        strcpy(buf_data[fill_here].buf,buf);
        buf_data[fill_here].filled=1;
        buf_data[fill_here].seqnum=seq_num;
        
        if(fill_here!=-1)
               {
                pack_data(prev_seq_num, MSG_TYPE_ACK, 0, ack_buf, 8);//CREATE THE ACK
                printf("Sending ACK FOR %d\n",prev_seq_num);
                sendto(serv.sock_server_recv, ack_buf, 8, 0, (struct sockaddr*)&sender, sizeof(sender));//SEND THE prev ACK
                }        
        
        }


printf("\n");

        }

	sock_status_recv = shutdown(serv.sock_server_recv,2);
  	if(sock_status_recv != 0) {
    		die("Error in socket termination for server: ", errno);
  	}
	close(serv.sock_server_recv);
	
	return 0;
}



