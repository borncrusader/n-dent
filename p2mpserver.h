#ifndef __P2MPSERVER_H__
#define __P2MPSERVER_H__

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include "p2mp.h"

typedef struct rev_window {
        struct node *left;
        struct node *right;
        struct node *head;
        struct node *toack;
} rev_window;

typedef struct p2mp_sb {
	int p;
	int N;
	int sock_server;
	char filename[FILE_NSIZE];
	struct sockaddr_in server;
	struct rev_window w;
} p2mp_sb;


typedef struct node {
	int seqnum;
	int filled;
	char buf[MSS];
	struct node *next;
	int ack;
}node;

#endif
