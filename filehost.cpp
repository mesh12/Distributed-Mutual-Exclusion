#include "main.h"

int count;

class Server
{

	int port; //port number of server`
	int id;
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
		printf("Starting server hosting files %d\n", sfd);

		bind(sfd, (struct sockaddr *) &serv_servaddr, sizeof(serv_servaddr));
		listen(sfd, 50);

		fd = sfd;
		printf("Started server hosting files\n");

	}



	public:

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

class Node
{
	int id;
	Server server;

	public:

	Node(int id, int server_port) : server(server_port)
	{
		this->id = id; //server id
	}

	void process_read(Message m)
	{
		std::ifstream f;
		std::string filename, dirname, ch;
		std::vector<std::string> line;

		filename = std::to_string(m.fileid);
		dirname = std::to_string(m.serverid);

		f.open(dirname+'/'+filename);

		if(f.is_open())
		{
			while(std::getline(f,ch))
			{
				line.push_back(ch);
			}
		}

		std::cout<<"Last line is: "<<line.back();
		f.close();


	}

	void process_write(Message m)
	{
		std::ofstream f;
		std::string filename,dirname;

		filename = std::to_string(m.fileid);
		dirname = std::to_string(m.serverid);

		f.open(dirname+'/'+filename);

		if(f.is_open())
		{
			f<<std::to_string(m.pid)<<" "<<std::to_string(m.timestamp)<<"\n";
		}

		f.close();

		std::cout<<"Written into file "<<m.fileid<<" "<<std::to_string(m.pid)<<" "<<std::to_string(m.timestamp)<<"\n";
	}

	void process_enquire(Message m)
	{
		DIR *d;
		struct dirent *p;
		std::string dirname;

		dirname = std::to_string(m.serverid);

		d = opendir(dirname);

		n = 0;
		if(d!=NULL)
		{
			while((p=readdir(d)))
			{
				n++;
			}
		}
		else
		{
			perror("Error: ");
		}

		closedir(d);

		count = n;

		std::cout<<"Number of files : "<<count<<"\n";
	}

	void process_message(Message m)
	{
		switch(m.o_type)
		{
			case READ: process_read(m);
				break;

			case WRITE: process_write(m);
				break;

			case ENQ : process_enquire(m);
				break;

			default: std::cout<<"I DO NOT UNDERSTAND THE MESSAGE";
				break;
		}
	}

} *node;

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
		m.msg_type = DONE;
		m.count = count;
		write(conn1,&m,sizeof(struct Message));
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
			//send file name and number of files

			pthread_create(&thread2,NULL,read_socket,&conn); // TODO
		}
	}

	pthread_join(thread2,NULL);

	return NULL;
}

int main(int argc, char *argv[])
{
	int sid;

	sid = atoi(argv[1]);
	node = new Node(sid, 1280);

	return 0;
}