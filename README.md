# RPC-and-Socket-Programming
Distributed string construction and property verification using Remote Procedure Calls and UDP socket in C

Usage: 
* ./server_verify
* ./server_append (server_verify hostname)
* ./client (property #) N L M c0 c1 c2 (server_append hostname) (server_verify hostname)

## Program function:
The program creates a string S with M segments, each of length L (|S| = M x L) using 3 <= N <= 8 threads.  
   
Each thread is assigned a unique character from {a, b, c, d, e, f, g} that is tries to append to S.  
   
The program accepts three characters (c0, c1, c2) as input arguments to participate in property verification.  
   
During each append, S is checked to ensure a certian property is met or can be met by a future append:
* Property 1: |c0| + |c1| = |c2|
* Property 2: |c0| + 2 x |c1| = |c2|
* Property 3: |c0| x |c1| = |c2|
* Property 4: |c0| - |c1| = |c2|
   
Upon completetion of S each segment is checked to ensure the property is met and the final string is printed.  
