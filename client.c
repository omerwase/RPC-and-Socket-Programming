/*
 * Programming Assignment 3 for CS 6F03 Winter 2017 Term
 * Filename: client.c
 * By: Omer Waseem (#000470449)
 * Date Submitted: Mar 21, 2017
 * Description: completed as per 100% grading criteria:
 * 	- Connects to append and verify servers through RPC
 *	- Launches N threads which call append
 *	- Once S is complete threads call getSeg and verify property
 *	- Master thread outputs result to console and file out.txt
 */


#include "verify.h"
#include "append.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

int thread_runner();
int checkSeg(char* seg);
void getFactor(int length);
char *S;
int M, L, N, i, count;
char c[3];
char alpha[] = {'a','b','c','d','e','f','g', 'h'};
int *flags;
int maxFactor[2];
CLIENT *appendHandle;
CLIENT *verifyHandle;

int main(int argc,char *argv[]) {
	
	char *host1;
	char *host2;
	int  *initAppend;
	append_param  appendP;
	int  *initVerify;
	verify_param  verifyP;
	FILE *f;
	
	if(argc != 10) {
		printf("usage: %s F N L M C0 C1 C2 host_name1 host_name2\n", argv[0]);
		exit(1);
	}
	
	// parse input paramters
	i = atoi(argv[1]);
	N = atoi(argv[2]);
	L = atoi(argv[3]);
	M = atoi(argv[4]);
	c[0] = argv[5][0];
	c[1] = argv[6][0];
	c[2] = argv[7][0];
	host1 = argv[8];
	host2 = argv[9];
	flags = malloc(sizeof(int*) * N);
	count = 0;
	maxFactor[0] = 0;
	maxFactor[1] = 0;
	
	// checks if input parameters make properties unenforcable
	if (N < 3 || N > 8) {
		printf("Invalid N value, 3 <= N <= 8\n");
		exit(0);
	}
	else if (N==3 && i==0 && L%2!=0) {
		printf("to enforce property F0, L must be even or N > 3\n");
		exit(0);
	} else if (N==3 && i==3 && L%2!=0) {
		printf("to enforce property F3, L must be even or N > 3\n");
		exit(0);
	}
	
	// check if solution for F2 exists with given length
	if (i == 2){
		getFactor(L);
	}

	printf("processing, please wait ...\n");
	
	
	// create client handle for append server
	appendHandle = clnt_create(host1, APPEND, APPEND_VER, "udp");
	if (appendHandle == NULL) {
		clnt_pcreateerror(host1);
		exit(1);
	}
	
	// create client handle for verify server
	verifyHandle = clnt_create(host2, VERIFY, VERIFY_VER, "udp");
	if (verifyHandle == NULL) {
		clnt_pcreateerror(host2);
		exit(1);
	}
	
	// init append server and send f, l, m, c0, c1, c2, host2
	appendP.f = i;
	appendP.l = L;
	appendP.m = M;
	appendP.c[0] = c[0];
	appendP.c[1] = c[1];
	appendP.c[2] = c[2];
	appendP.host_name2 = host2;
	appendP.maxFactor[0] = maxFactor[0];
	appendP.maxFactor[1] = maxFactor[1];
	initAppend = rpc_initappendserver_1(&appendP, appendHandle);
	if (initAppend == NULL) {
		clnt_perror(appendHandle, "failed to initiate append server");
	}
	
	// init verify server and send n, l, m
	verifyP.n = N;
	verifyP.l = L;
	verifyP.m = M;
	initVerify = rpc_initverifyserver_1(&verifyP, verifyHandle);
	if (initVerify == NULL) {
		clnt_perror(verifyHandle, "failed to initiate verify server");
	}
	
	// launch N local threads
	// each thread repeatedly tries to append a character, return 0:success, -1:complete, 1:property violation
	// threads that receive -1 call repeatedly getseg()
	// they verify property for segment and increment local counter
	// once getseg() returns "-" sum local counters
	omp_set_num_threads(N);
	#pragma omp parallel
	thread_runner();
	
	// master thread outputs S and the count to terminal
	// retrieves S from verify server
	S = *rpc_get_s_1(NULL, verifyHandle);
	printf("S: %s\ncount: %d\n", S, count);

	f = fopen("out.txt", "w");
	if (f == NULL) {
		printf("Error during output to out.txt\n");
	}
	else {
		fprintf(f, "%s\n", S);
		fprintf(f, "%d\n", count);
	}
	fclose(f);

	clnt_destroy(appendHandle);
	clnt_destroy(verifyHandle);
	printf("\n");
	return 0;
}

// thread function for appending characters and checking properties
int thread_runner () {
	int thread = omp_get_thread_num();
	char val = alpha[thread];
	int r, a, b, x = 0, sum = 0;
	int  *callAppend;
	int appendResult = 0, segFlag = 1;
	char **segResult;
	int localCount = 0;
	
	while (segFlag) {
		// sleep between 100ms and 500ms r = (rand() % 401) + 100;
		r = (rand() % 401) + 100; // random number between 100 and 500
		struct timespec t;
		t.tv_sec = 0;
		t.tv_nsec = r * 1000000; // conversion to nano seconds
		nanosleep(&t, NULL);
		
		// call append in OMP critical
		#pragma omp critical
		{
			callAppend = rpc_append_1(&val, appendHandle);
			appendResult = *callAppend;
		}
		
		if (callAppend == NULL) {
			clnt_perror(appendHandle, "call to rpc_append() failed");
		}
		else if (appendResult == 0) { // success appending character
			printf("thread %d slept for %dms successfully appended %c\n", thread, r, val);
			// reset flags when character appeneded
			for (x = 0; x < N; x++) {
				flags[x] = 0;
			}
		}
		else if (appendResult == 1) { // cannot append character
			// flags for tracking which threads result in violation of property
			flags[thread] = 1;
			printf("thread %d slept for %dms property violation %c\n", thread, r, val);
			// check how many threads result in violation of property
			for (x = 0, sum = 0; x < N; x++) {
				sum += flags[x];
			}
			// all threads result in violation of property
			if (sum == N){
				printf("string cannot be constructed with given parameters\n");
				exit(0);
			}
		}
		else if (appendResult == -1) { // S complete, start checking segment
			segResult = rpc_getseg_1(&thread, verifyHandle);
			if (segResult == NULL) {
				clnt_perror(verifyHandle, "call to rpc_getseg() failed");
			}
			if (*segResult[0] == '-') {
				segFlag = 0;
			} else {
				// check segment and increment localCount
				localCount += checkSeg(*segResult);
				printf("thread %d recieved segment %s\n", thread, *segResult);
			}
		}
	}
	

	// aggregate all the counts
	#pragma omp critical
	count += localCount;
	
	printf("thread %d localCount: %d\n", thread, localCount);
	
	return 0;
}

// function for checking segment properties
// input: segment to check
// output: flag = 0 if property is not satisfied, flag = 1 if property is satisfied
int checkSeg(char* seg) {
	int c0 = 0, c1 = 0, c2 = 0, flag = 0, x = 0;
	
	// loop through S to determine values of c0, c1, and c2
	for (x = 0; x < L; x++) {
		char val = *(seg + x);
		if (val == c[0]) {
			c0++;
		}
		else if (val == c[1]) {
			c1++;
		}
		else if (val == c[2]) {
			c2++;
		}
	}
	
	// flag to indicate property is met
	if (i == 0 && c0 + c1 == c2) {
		flag = 1;
	}
	else if (i == 1 && c0 + 2*c1 == c2) {
		flag = 1;
	}
	else if (i == 2 && c0 * c1 == c2) {
		flag = 1;
	}
	else if (i == 3 && c0 - c1 == c2) {
		flag = 1;
	}
	
	return flag;
}


// fuction that returns the maximum factor for verification of property F2
void getFactor(int length) {
	int i, j;

	for (i = 1; ; i += 1) {
		for (j = i; ; j+=1) {
			if (i * j + i + j == length) {
					maxFactor[0] = i;
					maxFactor[1] = j;
			}
			if (i * j + i + j >= length) {
				break;
			}
		}
		if (i >= j) {
			break;
		}
	}
}

