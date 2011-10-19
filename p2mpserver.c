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
	int sock_status;
	if(argc!=5)
		usage();
	else if(atoi(argv[4]) < 0 || atoi(argv[4]) > 1)
		usage();

	p2mp_sb serv;
	P2MP_ZERO(serv);
	
	serv.p = atoi(argv[4]);
	serv.N = atoi(argv[3]);
	strcmp(serv.filename, argv[2]);

	if((serv.sock_server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		die("Server Socket creation failed",errno);
		return errno;
	}
	
	serv.server.sin_family = AF_INET;
	serv.server.sin_addr.s_addr = INADDR_ANY;
	serv.server.sin_port= (htons(atoi(argv[1])));	

	if(bind(serv.sock_server, (struct sockaddr*)&serv.server, sizeof(serv.server)) == -1) {
		close(serv.sock_server);
		die("Server bind failed",errno);
		return errno;
	}
	
	sock_status = shutdown(serv.sock_server,2);
  	if(sock_status != 0) {
    		die("Error in socket termination for server: ", errno);
  	}
	close(serv.sock_server);
	
	return 0;
}

