/*
 * Programming Assignment 3 for CS 6F03 Winter 2017 Term
 * Filename: verify_server.c
 * By: Omer Waseem (#000470449)
 * Date Submitted: Mar 21, 2017
 * Description: completed as per 100% grading criteria:
 *	- recieves S from append server
 *	- sends segments to threads through getSeg
 */

#include "verify.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define VERIFYPORT 4522

static char *S;
int N, L, M, segNum;
int sock, slen, recvFlag;
struct sockaddr_in server;
struct sockaddr_in from;
char *Sbuf;
static char **segResult;

void cleanExit () {
	exit(0);
}

void error (char *msg){
	perror(msg);
	cleanExit();
}

int * rpc_initverifyserver_1_svc(verify_param *verifyP, struct svc_req *rqstp) {
	static int result = 0;;

	// initalize values and allocate memory
	segNum = 0;
	recvFlag = 0;
	N = verifyP->n;
	L = verifyP->l;
	M = verifyP->m;
	slen = (M * L + 1);
	S = malloc(sizeof(char*) * slen);
	S[0] = '\0';
	Sbuf = malloc(sizeof(char*) * slen);
	
	// initialize thread buffers for returning segment, 1 buffer for each thread
	segResult = malloc(sizeof(char*) * N);
	int x = 0;
	for (x = 0; x < N; x++){
		segResult[x] = malloc(sizeof(char*) * (L + 1));
	}
	
	// initalize UDP socket for receiving S
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	if (sock < 0) {
		error("socket failed on verify server\n");
	}
	int length = sizeof(server);
	bzero(&server, length);
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(VERIFYPORT);
	if (S[0] == '\0') {
		if (bind(sock, (struct sockaddr *)&server, length) < 0) {
			error("could not bind socket on verify server\n");
		}
	}
	
	printf("\nverify server initialized\n");
	
	return(&result);
}

char ** rpc_getseg_1_svc(int *thread, struct svc_req *rqstp) {
	// getseg() called for the first time
	if (recvFlag == 0) {
		int fromlen;

		// recieve S from append server using UDP socket
		fromlen = sizeof(struct sockaddr_in);
		bzero(Sbuf, slen);
		int n = recvfrom(sock, Sbuf, slen, 0, (struct sockaddr *)&from, &fromlen);
		if (n < 0 ) {
			error("could not recieve from append server\n");
		}
		strcpy(S, Sbuf);
		printf("recieved %s from append server\n", S);
		recvFlag = 1;
		close(sock);
	}

	// copy segments into result buffer for specific thread
	if (segNum < M) {
		strncpy(segResult[*thread], S + segNum*L, L);
		segResult[*thread][L] = '\0';
		segNum++;
		printf("sent segment %s to thread %d\n", segResult[*thread], *thread);
	} else { // no segments remain
		segResult[*thread] = "-";
	}
	
	// return buffer specific to thread
	return(&segResult[*thread]);
}

// to return S to client for printing
char ** rpc_get_s_1_svc(void *p, struct svc_req *rqstp) {
	return (&S);
}
