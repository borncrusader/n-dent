all: udpserver udpclient
	
udpserver:
	gcc -g udpserver.c common.c -o udpserver

udpclient:
	gcc -g udpclient.c common.c -o udpclient

clean:
	rm -rf udpserver udpclient
