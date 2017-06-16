# Makefile for PA3
all:
	rpcgen -C append.x
	rpcgen -C verify.x
	gcc -g -fopenmp -DRPC_SVC_FG -c -o client.o client.c
	gcc -g -DRPC_SVC_FG -c -o append_clnt.o append_clnt.c
	gcc -g -DRPC_SVC_FG -c -o append_xdr.o append_xdr.c
	gcc -g -DRPC_SVC_FG -c -o verify_clnt.o verify_clnt.c
	gcc -g -DRPC_SVC_FG -c -o verify_xdr.o verify_xdr.c
	gcc -g -fopenmp -DRPC_SVC_FG -o client append_clnt.o verify_clnt.o client.o append_xdr.o verify_xdr.o
	gcc -g -DRPC_SVC_FG -c -o append_svc.o append_svc.c
	gcc -g -DRPC_SVC_FG -c -o append_server.o append_server.c
	gcc -g -DRPC_SVC_FG -c -o verify_svc.o verify_svc.c
	gcc -g -DRPC_SVC_FG -c -o verify_server.o verify_server.c
	gcc -g -DRPC_SVC_FG -o server_append append_svc.o append_server.o append_xdr.o
	gcc -g -DRPC_SVC_FG -o server_verify verify_svc.o verify_server.o verify_xdr.o

clean:
	rm -f client server_append server_verify *.o append.h verify.h append_client.c append_clnt.c append_svc.c append_xdr.c verify_client.c verify_clnt.c verify_svc.c verify_xdr.c out.txt
