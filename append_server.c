/*
 * Programming Assignment 3 for CS 6F03 Winter 2017 Term
 * Filename: append_server.c
 * By: Omer Waseem (#000470449)
 * Date Submitted: Mar 21, 2017
 * Description: completed as per 100% grading criteria:
 *	- constructs S through append
 *	- checks append character for property violation
 *	- sends S to verify server once complete
 */


#include "append.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define VERIFYPORT 4522

int checkProp (char val);

int i, N, L, M, len, slen, maxFactor[2];
char c[3];
char *host2;
char *S;
int sentS;
int sock;
struct sockaddr_in server;
char *buffer;

void cleanExit() {
	exit(0);
}

void error (char *msg){
	perror(msg);
	cleanExit();
}

int * rpc_initappendserver_1_svc(append_param *appendP, struct svc_req *rqstp) {
	
	static int result = 0;
	struct hostent *hp;
	sentS = 0;
	
	// initalize parameters
	len = 0;
	i = appendP->f;
	N = appendP->n;
	L = appendP->l;
	M = appendP->m;
	c[0] = appendP->c[0];
	c[1] = appendP->c[1];
	c[2] = appendP->c[2];
	host2 = appendP->host_name2;
	maxFactor[0] = appendP->maxFactor[0];
	maxFactor[1] = appendP->maxFactor[1];
	slen = (M * L + 1);
	S = malloc(sizeof(char*) * slen);
	S[0] = '\0'; // initialize to empty string
	buffer = malloc(sizeof(char*) * slen);
	
	// setup UDP socket for sending S
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	signal(SIGTERM, cleanExit);
	signal(SIGINT, cleanExit);
	if (sock < 0) {
		error("socket failed on append server\n");
	}
	server.sin_family = AF_INET;
	hp = gethostbyname(host2);
	
	if (hp==0) {
		printf("unknown host: %s on append server\n", host2);
		cleanExit();
	}
	bcopy((char*)hp->h_addr, (char*)&server.sin_addr, hp->h_length);
	server.sin_port = htons(VERIFYPORT);
	
	printf("\nappend server initialized\n");
	
	return(&result);
}

// returns: 0 for success, -1 for S complete and 1 for property violation
int * rpc_append_1_svc(char *value, struct svc_req *rqstp) {
	
	static int result = 0;
	char val = *value;
	if (len < M * L) {
		// insert new value into S and only increment len if it doesn't volilate property
		S[len] = val;
		
		// CheckProp validates if val can be added to S without violating the property
		if (checkProp(val)){
			
			printf("appended %c at index %d\n", val, len);
			len++; // increase length count, confirming val as part of S
			result = 0;
			
			// check if len is reached and terminate string
			if (len == M * L) {
				S[len] = '\0';
				result = -1;
			}
		} else { // cannot append character val
			result = 1;
		}
	}
	
	if (result == -1 && sentS == 0) { // send S to verify server via UDP socket
		int length = sizeof(struct sockaddr_in);
		bzero(buffer, slen);
		strcpy(buffer, S);
		printf("S complete, sending %s to verify server\n", S);
		int n = sendto(sock, buffer, strlen(buffer), 0, &server, length);
		if (n < 0) {
			error("could not send S to verify server\n");
		}
		sentS = 1;
		close(sock);
	}
	return(&result);
}


// function to check if property can be enforced
// input: new value being inserted into S
// output: flag = 0 if value cannot be inserted, flag = 1 if value can be inserted
int checkProp (char val) {
	int c0 = 0, c1 = 0, c2 = 0, x = 0, flag = 0, c01 = 0;
	
	// free is the number of empty spaces left in the segment assuming new value is used
	int free = L - len % L - 1;
	
	// remaining free spaces after accounting for property satisfaction
	int remain;
	
	// loop through S to determine values of c0, c1, and c2
	for (x = len - (len % L); x <= len; x++) {
		if (S[x] == c[0]) {
			c0++;
		}
		else if (S[x] == c[1]) {
			c1++;
		}
		else if (S[x] == c[2]) {
			c2++;
		}
	}
	
	// check for F2 property if value being inserted is c0 or c1
	// if val makes both c0 and c1 greater than maxFactor, val cannot be inserted
	if (i == 2){
		
		if (maxFactor[0] == 0) {
			if (val == c[0] || val == c[2]){
				return 0;
			} else {
				return 1;
			}
		}
		
		if (val == c[0] && c0 > maxFactor[0]){
			return 0;
		}
		else if (val == c[1] && c1 > maxFactor[1])
		{
			return 0;
		}
	}
	
	if (i == 0) { // check for property F0
		// if property is met
		if (c0 + c1 == c2) {
			flag = 1;
		}
		else { // if property is not met, check if addition of new value leads to the property being unenforcable
			if (free == 0){
				flag = 0;
			}
			else { // determine if enough free spaces exist to accomodate future values required to satisfy property.
				if (c2 - c0 - c1 < 0 && free >= c0 + c1 - c2) {
					flag = 1;
				}
				else if (c2 - c0 - c1 > 0 && free >= c2 - c0 - c1) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
		}
	}
	else if (i == 1) { // check for property F1
		if (c0 + 2*c1 == c2) {
			flag = 1;
		}
		else {
			if (free == 0){
				flag = 0;
			}
			else {
				if (c0 + c1*2 - c2 > 0 && free >= c0 + c1*2 - c2) {
					remain = free - (c0 + c1*2 - c2);
					if (remain % 2 == 0 || remain % 3==0) {
						flag = 1;
					}
					else {
						flag = 0;
					}
				}
				else if (c2 - c0 - c1*2 > 0 && free >= c2 - c0 - c1*2) {
					remain = free - (c2 - c0 - c1*2);
					if (remain % 2 == 0 || remain % 3 == 0) {
						flag = 1;
					}
					else {
						flag = 0;
					}
				}
				else {
					flag = 0;
				}
			}
		}
	}
	else if (i == 2) { // check for property F2
		if (c0*c1==c2 && free==0) {
			flag = 1;
		}
		else {
			if (free == 0){
				flag = 0;
			}
			else {
				if (c0 == 0 || c1 == 0){
					c01 = c0 + c1;
					if (c01 >= c2 && free > c01 - c2) {
						flag = 1;
					}
					else if (c2 >= c01 && free > c2 - c01) {
						flag = 1;
					}
					else {
						flag = 0;
					}
				}
				else if (c2 - c0 * c1 < 0 && free >= c0 * c1 - c2) {
					flag = 1;
				}
				else if (c2 - c0 * c1 > 0 && free >= c2 - c0 * c1) {
					flag = 1;
				}
				else if (c2 == c0 * c1) {
					flag = 1;
				}
				else {
					flag = 0;
				}
				
				// if L < 3 and N > 3, then only non-property related characters can be used
				// or when L<3 and N==3 then only one of c[0] or c[1] must comprise all of S
				if (L<3 && N>3 && val!=c[0] && val!=c[1] && val!=c[2]) {
					flag = 1;
				}
				else if (L<3 && N==3){
					if (len==0) {
						if (val != c[2]){
							flag = 1;
						}
						else {
							flag = 0;
						}
					}
					else {
						if (val==S[0]){
							flag = 1;
						}
						else {
							flag = 0;
						}
					}
				}
				else if (L<3 && N>3) {
					flag = 0;
				}
			}
		}
	}
	else if (i == 3) { // check for property F3
		if (c0 - c1 == c2) {
			flag = 1;
		}
		else {
			if (free == 0){
				flag = 0;
			}
			else {
				if (c2 - (c0 - c1) < 0 && free >= (c0 - c1) - c2) {
					flag = 1;
				}
				else if (c2 - (c0 - c1) > 0 && free >= c2 - (c0 - c1)) {
					flag = 1;
				}
				else {
					flag = 0;
				}
			}
		}
	}
	
	return flag;
}

