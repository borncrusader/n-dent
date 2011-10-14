CC        =   gcc
CPPFLAGS  =   -g -DDEBUG
LIB       =   -lpthread
P_CLIENT  =   rdt_send.o send_thread.o receive_thread.o p2mpclient.o
P_SERVER  =   p2mpserver.o
SRC       =   rdt_send.c send_thread.c receive_thread.c p2mpclient.c
H_FILES   =   p2mp.h p2mpclient.h

all: ccunix p2mpclient

ccunix:
	$(CC) $(CPPFLAGS) -c $(SRC)

p2mpclient:
	$(CC) $(CPPFLAGS) $(LIB) $(P_CLIENT) -o p2mpclient

p2mpserver:
	$(CC) $(CPPFLAGS) $(LIB) $(P_SERVER) -o p2mpserver

clean:
	rm -rf $(P_CLIENT) $(P_SERVER) p2mpclient p2mpserver
