#include "dfifo.h"

Dfifo* dfifo_init(char* id){
	DFifo* this;
	if( ((this) = (DFifo*) malloc(sizeof(DFifo)) ) == NULL){
		return NULL;
	}
	this->fifo = fifo_init(1);
	memcpy(this->id,id,(IDLENGTH>strlen(id))? strlen(id) : IDLENGTH );
	
	if(this->fifo==NULL){
		free(this);
		return NULL;
	}
	
	struct sched_param param;
	pthread_attr_init(&this->server->attr);
	pthread_attr_setschedpolicy(&this->server->attr,SCHED_RR);
	param.sched_priority = 98;
	pthread_attr_setschedparam(&this->server->attr,&param);
	pthread_create(&this->server->tid,&this->server->attr,&dfifo_server,(void *) this );
	
	
	
	return NULL;
}
void* dfifo_server(void* args){
	DFifo* this = (Dfifo*) args;
	int listenfd;
	pthread_t tid;
	socklen_t clilen;
	struct sockaddr_in servaddr,cliaddr;
	int serverport = 4444;
	listenfd = socket(AF_INET, SOCK_STREAM,0);
	
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(serverport);
	
	if(bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))!=0){
		perror("bind error");
	}
	
	listen(listenfd, LISTENQ);
	
	signal(SIGINT, sig_int);
	int keepServerRunning=1;
	pthread_attr_t attr;
	struct sched_param param;
	pthread_attr_init(&attr);
	pthread_attr_setschedpolicy(&attr,SCHED_FIFO);
	param.sched_priority = 98;
	pthread_attr_setschedparam(&attr,&param);
	clilen = sizeof(cliaddr);
	while(keepServerRunning){		
		Connection* conn = (Connection*) malloc(sizeof(Connection));
		conn->dfifo = this;
		do{
			conn->fd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);				
		}while( conn->df < 0 );
		pthread_create(&tid,&attr,&dfifo_server_child,(void *) conn);
	}
	
	close(listenfd);	
	
	return NULL;
}
void* dfifo_server_child(void* args){
	Connection* conn = (Connection*) args;
	Dfifo* this = (Dfifo*) conn->dfifo;
	char buffer[BUFFLENGTH];
	char cmd[BUFFLENGTH];
	char result[BUFFLENGTH];
	size_t size;
	void* data=NULL;
	bzero(buffer,BUFFLENGTH);
	
	if(NULL==(conn->in=fdopen(conn->fd,"r"))||NULL==(conn->out=fdopen(conn->fd,"w"))){
		perror("fdopen error");		
	}
	setlinebuf(out);	
	int keepAlive = 1;
	while(keepAlive){
		//keepAlive = 0;
		if(fgets(buffer,BUFFLENGTH,conn->in)==NULL){
			printf("closing connection: NULL data\n");
			break;
		}
		if(sscanf(buffer,"%s",&cmd)<1){ //there can be string problem here
			break;
		}
		if(strcmp(cmd,"PUSH",strlen("PUSH"))==0){
			if(fgets(buffer,BUFFLENGTH,conn->in)==NULL){
				printf("closing connection: NULL data\n");
				break;
			}
			if(sscanf(buffer,"%ld",&size)<1){				 
				break;
			}
			data = malloc(size);
			if(data==NULL){
				break;
			}
			bzero(data,size);
			
			if(fread(data,size,1,conn->in)<1){
				printf("closing connection: no data\n");
				break;
			}
			
			if(fifo_push(this->fifo,data,size)==0){
				if(fputs("OK\n",conn->out)<0){
					break;
				}
			}else{
				if(fputs("NOK\n",conn->out)<0){
					break;
				}	
			}
			
			free(data);	
		}else if(strcmp(cmd,"POP",strlen("POP"))==0){
			if(fifo_pop(this->fifo,&data,&size)==-1){
				printf("Problem in pop\n");
				break;
			}
			if(sprintf(buffer,"%ld\n",size)<0){
				break;
			}
	
			if(fputs(buffer,out)<0){
				break;
			}
			
			if(fwrite(data,size,1,conn->out)<1){
				printf("closing connection: no data\n");
				break;
			}
			if(fgets(buffer,BUFFLENGTH,conn->in)==NULL){
				printf("closing connection: NULL data\n");
				break;
			}
			if(strcmp(buffer,"OK",strlen("OK"))==0){
				free(data);				
			}else{
				if(fifo_push(this->fifo,data,size)==0){
					if(fputs("RETRY\n",conn->out)<0){
						break;
					}
				}
				free(data);				
			}							
		}else if(strcmp(cmd,"SIZE",strlen("SIZE"))==0){
			if(sprintf(buffer,"%d\n",fifo_size(this->fifo))<0){
				break;
			}
			if(fputs(buffer,conn->out)<0){
				break;
			}			
		}
		
		
		
	}	
	free(data);
	close(conn->fd);
	
	return NULL;
}
/*Listens for UDP packets*/
void* dfifo_listener(void* args){
	return NULL;
}
/*if fifo is flooded send udp help message*/
void* dfifo_watcher(void* args){
	return NULL;
}

void* dfifo_client(void* args){

	return NULL;
}
