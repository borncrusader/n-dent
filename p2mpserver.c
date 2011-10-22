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

	int sock_status_recv;
	char buf[BUFFER_SIZE];
	
	int ret = 0, seq_num = 0, type = 0, flag = 0,flags=0,prev_seq_num=-1;

	char from[INET_ADDRSTRLEN];
	FILE *fp;

	

	if(argc!=5) {
		usage();
	}
	else if(atof(argv[4]) >= 0 || atof(argv[4]) <= 1) {
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



while(flag)
{

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

	printf("Seq number got is %d \n",seq_num);


if(seq_num==prev_seq_num+1)
{

fwrite(buf+HEADER_SIZE,ret-HEADER_SIZE,1,fp);
//sendack(seq_num,serv,sender,len);


pack_data(seq_num, MSG_TYPE_ACK, 0, ack_buf, 8);


sendto(serv.sock_server_recv, ack_buf, 8, 0, (struct sockaddr*)&sender, sizeof(sender));

}
}

	sock_status_recv = shutdown(serv.sock_server_recv,2);
  	if(sock_status_recv != 0) {
    		die("Error in socket termination for server: ", errno);
  	}
	close(serv.sock_server_recv);
	
	return 0;
}









/*

int sendack(int seq_num,struct p2mb_sb serv ,struct sockadd_in sender,socklen_t len)
{

char ack_buf[8];

pack_data(seq_num, MSG_TYPE_ACK, 0, ack_buf, 8);

	if((serv.sock_server_send = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("Server ACK Socket creation failed",errno);
		return errno;
	}
	

	if(bind(serv.sock_server_send, (struct sockaddr*)&serv.server, sizeof(serv.server)) == -1) {
		close(serv.sock_server_send);
		die("Server ACK bind failed",errno);
		return errno;
	}

sendto(serv.sock_server_send, ack_buf, 8, 0, (struct sockaddr *)&sender, sizeof(struct sockaddr_in));





}


int get_prob()
{
int rand_val;

rand_val=(rand()%1000)/1000

return rand_val;

}
*/
