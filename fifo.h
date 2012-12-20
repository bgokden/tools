#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <string.h>

typedef struct FifoElement FifoElement;

struct FifoElement{
	void* data;
	size_t size;
	FifoElement* next;	
};

typedef struct Fifo {
	FifoElement* head;
	FifoElement* tail;
	sem_t lock;	
	int pshared;
	sem_t size;
}Fifo;
Fifo* fifo_init(int pshared);
void fifo_destroy(Fifo* this);
int fifo_push(Fifo* this,void* args,size_t size);
int fifo_pop(Fifo* this,void** args,size_t* size);
int fifo_try_pop(Fifo* this,void** args,size_t* size);
int fifo_size(Fifo* this);


