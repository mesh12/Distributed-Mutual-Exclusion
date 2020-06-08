
/*Required Headers*/
#include "main.h"

int replies[4];
// using namespace std;

class Server
{

	int port; //port number of server`
	//int id;
	int fd;
	pthread_t thread;

	struct sockaddr_in serv_servaddr;

	void start_server()
	{
		int sfd;

		sfd = socket(AF_INET,SOCK_STREAM,0);

		bzero(&serv_servaddr, sizeof(serv_servaddr));

		serv_servaddr.sin_family = AF_INET;
		serv_servaddr.sin_addr.s_addr = htons(INADDR_ANY);
		serv_servaddr.sin_port = htons(port);
		printf("Starting server %d\n", sfd);

		//inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
		bind(sfd, (struct sockaddr *) &serv_servaddr, sizeof(serv_servaddr));
		listen(sfd, 50);

		fd = sfd;
		printf("Started server\n");

	}



	public:
	
	int id;

	void join()
	{
		pthread_join(thread, NULL);
	}

	Server(int port)
	{
		this->port = port;
		this->id = port;
		start_server();
		pthread_create(&thread, NULL, acceptor, &fd);
	}
};



class Client
{
	int fd;
	struct sockaddr_in servaddr;
	char *ip;

	void start_client()
	{
		
		int y = 0;

		int a = -1;

		printf("Trying to connect to %d\n", port);
		while(a==-1)
		{
			if((fd = socket(AF_INET, SOCK_STREAM, 0))==-1)
			{
				//printf("Error on socket\n");
				//perror("Error on socket creation:");
				y = y + 1;
			}
			inet_pton(AF_INET,"127.0.0.1",&(servaddr.sin_addr));
			servaddr.sin_family = AF_INET;
			servaddr.sin_port = htons(port); //port number of the server the client wants to connect to
			if((a=connect(fd,(struct sockaddr *)&servaddr,sizeof(servaddr)))==-1)
			{
				//printf("Error on connect\n");
				//perror("Error on connect: ");
				y = y+1;
			}
		}
		printf("Connected to %d\n", port);
		

	}

	public:
	int id;

	void send_message(Message msg)
	{
		// struct Message msg;
		//printf("TEST %d\n", fd);
		char str[100];
		int s,i;
		msg.pid = id;
		//write(fd,arr[k],strlen(arr[k])+1);
		//write(fd,id,strlen(id)+1);
		i=send(fd, &msg,sizeof(struct Message),0);
		printf("\nSent to server %d is %d : ",port,msg.pid);
		print_message(msg);
		perror("Send errors ");
		// read(fd,(void *)&s,sizeof(int));
		// replies[k] = s;

		//printf("recieved from server: %d\n",s);
		//printf("Count = %d\n",count);

	}

	Client(int id, char *ip)
	{
		this->id = id;
		this->ip = ip;
		start_client();
	}
};


class Node
{
	Server server;
	int id, timestamp,max,rand_serverid;
	std::vector<Client> clients;
	std::vector<Client> servers;
	std::vector<std::queue<Message> > q; 
	std::vector<std::set<int> > reply_set;

	public:
	Node(int id, int server_port) : server(server_port)
	{
		timestamp = 1;
		this->id = id; //server id
	}

	int get_timestamp()
	{
		return timestamp;
	}

	void add_client(int port) //object with server and all clients
	{
		std::cout << "Adding a client to port "<<port<<"\n";
		clients.push_back(Client(id, port));
	}

	void add_server(int port)
	{
		std::cout<<"Adding a server to port "<<port<<"\n";
		servers.push_back(Client(id,port)); //port should be ip address

	}

	void critical_section(Message msg)
	{
		int i;
		
		if(msg.o_type==READ)
		{
			servers[msg.serverid-1].send_message(msg);
		}

		else
		{
			for(i=0;i<num_servers;i++)
			{
				servers[i].send_message(msg);
			}
		}

	}

	std::set<int> check_replies(Message msg)
	{
		std::set<int> result;
		if(reply_set[msg.fileid].empty())
		{
			return std::set<int>();
		}

		else
		{
			std::set_difference(process_id_set.begin(),process_id_set.end(),reply_set[msg.fileid].begin(),reply_set[msg.fileid].end(),std::inserter(result,result.end()));
			return result;
		}
	}

	//Function to process all request messages
	void process_request(Message msg)
	{
		int i;
		max = std::max(msg.timestamp,timestamp);
		timestamp = max;
		max++;
		//if requesting client has a lower timestamp or current client has no request
		if(((current.msg_type==REQ)&&(msg.timestamp<current.timestamp))||(current.msg_type==NONE))
		{

			Message m1;
			m1.pid = id;
			m1.msg_type = REP;
			m1.serverid = msg.serverid;
			m1.fileid = msg.fileid;
			m1.timestamp = max;
			for(i=0;i<clients.size();i++)
			{
				if(msg.pid==clients[i].id)
				{
					send_message(clients[i],m1); //send a reply message back to client 
				}
			}
			
			
		}

		else if((current.msg_type==REQ) && (msg.timestamp>current.timestamp))
		{
			q[msg.fileid-1].push(msg);

		}


		else
		{
			if(msg.pid>id)
			{
				q[msg.fileid-1].push(msg);
			}
			else
			{
				Message m1;
				m1.pid = id;
				m1.msg_type = REP;
				m1.serverid = msg.serverid;
				m1.fileid = msg.fileid;
				m1.timestamp = max;
				for(i=0;i<clients.size();i++)
				{
					if(msg.pid==clients[i].id)
					{
						send_message(clients[i],m1); //send a reply message back to client 
					}
				}
			}
		}

	}

	//Function to process all reply messages
	void process_reply(Message msg)
	{
		reply_set[msg.fileid-1].insert(msg.pid);
		timestamp = std::max(msg.timestamp,timestamp);
		if(reply_set[msg.fileid-1].size()==(num_clients-1))
		{
			std::cout<<"All replies recieved\n";
			rep_flag = 1 ;
			critical_section(current);
		}
	}

	void process_exit(Message msg)
	{
		Message m1;
		int i,j;

		num_files = msg.count;
		current.msg_type = NONE;
		timestamp++;
		//send all requests in queue
		if(!q[msg.fileid].empty())
		{
			rep_flag = 0;
			for(i=0;i<q[msg.fileid-1].size();i++)
			{
				m1 = q[msg.fileid-1].front();
				q[msg.fileid-1].pop();
				reply_set[msg.fileid-1].erase(msg.pid);
				msg.msg_type = REP;
				msg.timestamp = timestamp;
				for(j=0;j<clients.size();j++)
				{
					if(m1.pid==clients[j].id)
					{
						send_message(clients[j],msg);
					}	
				}
				
			}
		}
	}

	//Handler to process all messages recieved from clients
	void process_message(Message msg)
	{
		//timestamp = std::max(msg.timestamp, timestamp);
		switch(msg.msg_type)
		{
			case REQ: process_request(msg);
						
				break;
			case REP: process_reply(msg);

				break;

			case DONE: process_exit(msg);
				break;
			default:
				printf("I DO NOT KNOW THIS MESSAGE\n");
				break;
		}
	}

	void handler(Message msg,int flag)
	{
		if (flag==0)
		{	
			servers[0].send_message(msg);
			timestamp++;
		}

		else if(flag==1)
		{
			std::set<int> s;
			std::vector<int> v;
			int j;
			s = check_replies(msg);
			if(!s.empty())
			{
				std::copy(s.begin(), s.end(), std::back_inserter(v));
				for(int i=0;i<v.size();i++)
				{
					for(j=0;j<clients.size();j++)
					{	
						if(v[i]==clients[j].id)
						{
							send_message(clients[j],msg);
						}
					}
					
				}
			}
			else
			{
				broadcast_message(msg);
			}


					
		}

		else if(flag==2)
		{

		}
	}

	void send_message(Client client, Message msg)
	{
		if(rep_flag==0)
			msg.timestamp = timestamp;
		timestamp++ ;
		if(msg.msg_type==REQ)
			current = msg;
		client.send_message(msg);
	}

	void broadcast_message(Message msg) //send request to all servers
	{
		// std::cout << "Broadcasting message \n";
		print_message(msg);
		for (int k = 0; k < clients.size(); k++) 
		{
			msg.index = k;
			send_message(clients[k], msg);
		}
	}

	void join()
	{
		server.join();
	}
}*node;


void print_message(Message msg)
{
	if(msg.pid>0)
	{
		printf("{ %d %d %d %d }\n", msg.msg_type, msg.pid, msg.timestamp, msg.index);
	}
}

void *read_socket(void *conn)
{
	struct Message m;
	int conn1,s;

	conn1 = *(int *)conn;
	while(1)
	{
		if((recv(conn1,(void *)&m,sizeof(struct Message),0))<0)
		{
			perror("Receive errors ");
		}

		printf("Received message - %d : ",(int)m.pid);
		print_message(m);
		//printf("Errors will follow\n");

		node->process_message(m);
	}

	return NULL;

}

void *acceptor(void *fd)
{
	int conn;
	char str[100];
	struct Message msg; 
	pthread_t thread2;
	printf("Accepting connection %d\n", *(int *)fd);

	while(1)
	{
		conn = accept(*(int *)fd, (struct sockaddr*) NULL, NULL);
		printf("Accepted connection \n");

		printf("Connected\n");
		if(conn!=-1)
		{
			pthread_create(&thread2,NULL,read_socket,&conn); // TODO
		}
	}

	pthread_join(thread2,NULL);

	return NULL;
}

//command line arguments - pid, 5 clients, 3 servers
int main(int argc, char *argv[]) //pass server names
{
	int sfd,i,port_number[5],server_ip[3],process_id;
	std::set<int> s;
	// count = 0;
	srand(time(NULL));


	n = argc - 1;
	num_clients = argc - 1 - 3;
	num_servers = 3;

	printf("%d",n);

	process_id = atoi(argv[1]); //assign process id of the client

	for(i=n-1;i>n-1-3;i--)
	{
		server_ip[i] = atoi(argv[i]);
	}

	for(i=1;i<n-3;i++) // first port number is client, remaining 4 are server
	{
		port_number[i-1] = atoi(argv[i]);
		process_id_set.insert(i);
	}

	process_id_set.erase(port_number[0]);
	
	// sfd = start_server(client);
	node = new Node(process_id, port_number[0]);

	//connect to server host to get no of files


	for(i=0;i<num_clients-1;i++)
	{
		std::cout << "HERE\n";
		node->add_client(port_number[i]);
	}

	for(i=0;i<num_servers;i++)
	{
		node->add_server(server_ip[i]);
	}

	Message msg;
	msg.o_type = ENQ;
	node->handler(msg,0);

	int random_int,t,k;

	t = node->get_timestamp();
	while(t!=5)
	{
		random_int = 1+rand()%10;
		sleep(random_int);
		msg.pid = process_id;
		k = 1 + rand()%2;
		if(k==1)
		{
			msg.o_type = READ;
			k = 1 + rand()%3;
			msg.serverid = k;
		}
		else
		{
			msg.o_type = WRITE;
		}
		msg.fileid = rand()%num_files;
		msg.msg_type = REQ;
		node->handler(msg,1);
		
		t = node->get_timestamp();
	}

	//msg.msg_type = REQ;
	//node->broadcast_message(msg);


	node->join();

	return 0;
}






