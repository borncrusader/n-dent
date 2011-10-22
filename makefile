CC        =   gcc
CPPFLAGS  =   -g -DDEBUG
LIB       =   -lpthread
P_CLIENT  =   p2mp.o rdt_send.o send_thread.o receive_thread.o timer_thread.o p2mpclient.o
P_SERVER  =   p2mp.o p2mpserver.o
SRC       =   p2mp.c rdt_send.c send_thread.c receive_thread.c timer_thread.c p2mpclient.c p2mpserver.c
H_FILES   =   p2mp.h p2mpclient.h p2mpserver.h

all: ccunix p2mpclient p2mpserver

ccunix:
	$(CC) $(CPPFLAGS) -c $(SRC)

p2mpclient:
	$(CC) $(CPPFLAGS) $(LIB) $(P_CLIENT) -o p2mpclient

p2mpserver:
	$(CC) $(CPPFLAGS) $(LIB) $(P_SERVER) -o p2mpserver

clean:
	rm -rf $(P_CLIENT) $(P_SERVER) p2mpclient p2mpserver
