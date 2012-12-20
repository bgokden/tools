#include "fifo.h"

/*****Distributed Fifo********/
#define IDLENGTH 100
#define LISTENQ 1024
#define BUFFLENGTH 1024
struct Thread_data{
	pthread_t tid;
	void* data;
	int keepRunning;
	pthread_attr_t attr;
} Thread_data;

typedef struct Dfifo_id{
	char[IDLENGTH] id;
	char[IDLENGTH] ip;
	int port;
}

typedef struct Dfifo {
	Fifo* fifo;
	Thread_data server;
	Thread_data listener;
	
}Dfifo;

typedef struct Connection{
	int fd;
	FILE *in,*out;
	Dfifo* dfifo;
	int ReaderAlive;
	int WriterAlive;
} Connection;

Dfifo* dfifo_init(char* id,char* ip,int port);
void* dfifo_server(void* args);
void* dfifo_server_child(void* args);
void* dfifo_listener(void* args);
void* dfifo_watcher(void* args);
void* dfifo_client(void* args);
