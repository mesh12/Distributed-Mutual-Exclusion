#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <vector> 
#include <iostream> 
#include <iterator>
#include <queue>
#include <set>
#include <fstream>
#include <string>
#include <algorithm>

enum MessageType {REQ = 0, REP = 1, DONE = 2, NONE = 5};
enum Operation {ENQ = 0, READ = 1, WRITE =2};

struct Message
{
	enum MessageType msg_type;
	enum Operation o_type;
	int pid;
	char* host; //ip address or hostname
	int timestamp;
	int index;
	int count;
	int fileid; //1,2,3
	int serverid;//1,2,3

	Message():msg_type(NONE)
	{

	}
};

struct socket_ptr
{
	int fd;
	struct sockaddr_in servaddr;
};

// int start_server(int);
// void start_client(int[],int,int);
// void send_message(int, struct sockaddr_in,int,int,int,int);

void *acceptor(void *);
void *read_socket(void *);
void print_message(Message);

struct socket_ptr ptr[5];
int n;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
Message current; //gloabl structure to store the message type if a client wants to access CS
std::set<char*> process_id_set;
int num_clients,num_servers, num_files, rep_flag = 0;
