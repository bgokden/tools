#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <sys/mman.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/mman.h>
#include <math.h>

#include "fifo.h"

#define timespec_time2sec(t) ( t.tv_sec + (double) t.tv_nsec / 1000000000 ) 

#define timespec_sub(rem,op2,op1) (rem)->tv_sec = (op2)->tv_sec - (op1)->tv_sec; \
					if((rem)->tv_sec>0) \
						(rem)->tv_nsec = 1000000000 - (op1)->tv_nsec + (op2)->tv_nsec + 1; \
					else \
						(rem)->tv_nsec = (op2)->tv_nsec - (op1)->tv_nsec;

#define PRE_ALLOCATION_SIZE (1000*1024*1024)
#define SMALL_STACK_SIZE (500*1024)
void configure_malloc_behavior();
void reserve_process_memory(int size);

#define REPEAT 1000000

typedef struct r_total{
	double value;
} r_total;

void* pusher(void * arg){
	struct sched_param main_param;
    main_param.sched_priority = 98;
    sched_setscheduler(getpid(),SCHED_RR,&main_param);


	Fifo* fifo = (Fifo*) arg;
	char data[50];
	sprintf(data,"test");
	int i=REPEAT;
	double total=0;
	struct timespec then_time,now_time,rem_time;
	while(i--){
		sprintf(data,"testi %d",REPEAT-i);
		//printf("pusher %d data %s size: %d ,fifo size: %d\n",REPEAT-i,data,sizeof(data),fifo_size(fifo));
		clock_gettime(CLOCK_REALTIME, &now_time);		
		if(fifo_push(fifo,data,sizeof(data))==-1)
			printf("Problem in push\n");
		clock_gettime(CLOCK_REALTIME, &then_time);
		timespec_sub(&rem_time,&then_time,&now_time);
		total+=timespec_time2sec(rem_time);
	}
	
	//printf("push time average : %lf sec, total time: %lf sec\n,size: %d\n",total /REPEAT,total,fifo_size(fifo));
	
	r_total* val = (r_total*) malloc(sizeof(r_total));
	val->value = total;
	return val;
}

void* popper(void * arg){
	struct sched_param main_param;
    main_param.sched_priority = 98;
    sched_setscheduler(getpid(),SCHED_RR,&main_param);


	Fifo* fifo = (Fifo*) arg;
	void* data;
	char scan[100];	
	int i=REPEAT;
	double total=0;
	size_t size=0;
	struct timespec then_time,now_time,rem_time;
	int check=0;
	int n = 0;
	while(i--){
		clock_gettime(CLOCK_REALTIME, &now_time);
		if(fifo_pop(fifo,&data,&size)==-1)
			printf("Problem in pop\n");
		clock_gettime(CLOCK_REALTIME, &then_time);
		timespec_sub(&rem_time,&then_time,&now_time);
		total+=timespec_time2sec(rem_time);
		((char*)data)[size+1] = '\0';
		if((n=sscanf((char*)data,"%s %d",(char *)&scan,&check))<2){
			printf("Read n:%d,scan %s,check %d\n",n,(char *)&scan,check);
		}
		if(check != REPEAT-i){
			printf("Problem at %d for data %s with check : %d\n",REPEAT-i,(char *)data,check);
		}	
		//printf("popper %d data %s size: %d ,fifo size: %d\n",REPEAT-i,data,size,fifo_size(fifo));
		free(data);
	}
	
	//printf("pop time average : %lf sec, total time : %lf sec\n",total /REPEAT,total);
	r_total* val = (r_total*) malloc(sizeof(r_total));
	val->value = total;
	return val;
}



int main(int argc, char* argv[])
{
	configure_malloc_behavior();
	reserve_process_memory(PRE_ALLOCATION_SIZE);	
	
	Fifo* fifo = fifo_init(0);
	
	if(fifo==NULL){
		printf("cannot create\n");
		return 1;
	}
	
	if(fifo==NULL){
		printf("problem\n");
		return 1;
	}
			
	pthread_t tid;
	pthread_create(&tid,NULL,(void*)&pusher,fifo);
	
	
	pthread_t tid1;
	pthread_create(&tid1,NULL,(void*)&popper,fifo);
	
	void* val;
	void* val1;
	
	pthread_join(tid,&val);
	pthread_join(tid1,&val1);		

	double total = ((r_total*)val)->value;
	printf("push time average : %lf sec, total time: %lf sec\n",total /REPEAT,total);
	total = ((r_total*)val1)->value;
	printf("pop time average : %lf sec, total time : %lf sec\n",total /REPEAT,total);
	printf("size left: %d\n",fifo_size(fifo));
		
	fifo_destroy(fifo);
	
	return 0;
}

void configure_malloc_behavior()
{
	if(mlockall(MCL_CURRENT | MCL_FUTURE))
		perror("mlockall failed:");
	
	mallopt(M_TRIM_THRESHOLD, -1);
	
	mallopt(M_MMAP_MAX, 0);	
}

void reserve_process_memory(int size)
{
	int i;
	char *buffer;
	buffer = malloc(size);
	
	for (i=0; i < size ; i+= sysconf(_SC_PAGESIZE)){
		buffer[i]=0;
	}
	
	free(buffer);
}

