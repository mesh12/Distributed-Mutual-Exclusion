# Distributed-Mutual-Exclusion
Implementation of a client-server model to read and write files in a multi machine environment 

This system consists of 3 servers and 5 clients.
The servers contain files which can be read from or written into by clients.
All files are replicted on the server. A READ/WRITE for a particiular file can be accessed by a single client. READ/WRITE
can be concurrently performed by different clients for different files.
This follows the Ricart-Agarwal algorithm to allow multiple clients to access a resource to ensure no violations in 
READ/WRITE operations occur and to prevent deadlocks

main.c comprises the code for all clients as well as the algorith, implementaion 
filehost.cpp is the code to start up the servers.
