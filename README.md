# RPC-and-Socket-Programming
Distributed string construction and property verification using Remote Procedure Calls and UDP socket in C

Usage: requires three computers (client, append server and verify server)
* ./server_verify
* ./server_append (server_verify hostname)
* ./client (property #) N L M c0 c1 c2 (server_append hostname) (server_verify hostname)

## Program function:
### Client:
Creates 3 <= N <= 8 threads using OpenMP, each of which is assigned a unique character from {a, b, c, d, e, f, g}.  
Each thread sends its character to the append server to construct S, which is composed of M segements of length L (|S| = M x L).    
The client accepts three characters (c0, c1, c2) as input arguments to participate in property verification.  
Once S is fully constructed, each thread obtains a segment from the verify server to check the property indeed holds. 
### Append Server    
Once a character is recieved from the client, the server constructs S, checking that a certian property is met or can be met by a future append:
* Property 1: |c0| + |c1| = |c2|
* Property 2: |c0| + 2 x |c1| = |c2|
* Property 3: |c0| x |c1| = |c2|
* Property 4: |c0| - |c1| = |c2|
   
Upon completetion of S the entire string is sent to the verify server via UDP socket
### Verify Server
After recieving S from the append server, the verify server sends M segments of S to the client upon recieving a request through RPC.  
If no more segments are left, the verify server returns "-" indicating end of the string to the client.  
