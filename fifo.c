#include "fifo.h"

Fifo* fifo_init(int pshared){
	Fifo* this;
	if( ((this) = (Fifo*) malloc(sizeof(Fifo)) ) == NULL){
		return NULL;
	}
	bzero(this,sizeof(Fifo));
	if((sem_init(&(this)->size,pshared,0))==-1){ 
		free(this);
		return NULL;
	}
	if((sem_init(&(this)->lock,pshared,1))==-1){ 
		free(this);
		return NULL;
	}
	(this)->pshared = pshared;
	return this;
}
void fifo_destroy(Fifo* this){
	void* temp=NULL;
	size_t size;
	while(fifo_try_pop(this,&temp,&size)==0)
	{
		free(temp);
	}
	sem_destroy(&(this)->size);
	sem_destroy(&(this)->lock);
	free(this);
}
int fifo_push(Fifo* this,void* args,size_t size){

	if(this==NULL){
		return -1;
	}
	
	
	if(args==NULL){
		return -1;
	}
	FifoElement* node;
	if((node=(FifoElement*) malloc(sizeof(FifoElement)))==NULL){
		return -1;
	}
	bzero(node,sizeof(FifoElement));

	node->size = size;
	if((node->data= malloc(node->size))==NULL){
		free(node);
		return -1;
	}
	
	memcpy(node->data,args,node->size);

	/**********lock************/
	if(sem_wait(&this->lock)==-1){
		//printf("Problem");
		return -1;
	}
	/**************************/

	if(this->head==NULL){
		this->head = node;		
	}else{	
		this->tail->next = node;
	}
	this->tail = node;
	//printf("push %d\n",i++);
	/**********unlock***********/
	if(sem_post(&this->lock)==-1){
		return -1;
	}
	/**************************/
	
	if(sem_post(&this->size)==-1){
		return -1;
	}

	return 0;

}
int fifo_pop(Fifo* this,void** args,size_t* size){

	if(this==NULL){
		return -1;
	}

	if(sem_wait(&this->size)==-1){
		return -1;
	}

	/**********lock************/
	if(sem_wait(&this->lock)==-1){
		return -1;
	}
	/**************************/

	if(this->head==NULL){
		return -1;
	}
	*size = this->head->size;

	*args = this->head->data;

	this->head = this->head->next;

	/**********unlock************/
	if(sem_post(&this->lock)==-1){
		return -1;
	}
	/**************************/
	
	return 0;
}

int fifo_try_pop(Fifo* this,void** args,size_t* size){

	if(this==NULL){
		return -1;
	}

	if(sem_trywait(&this->size)==-1){
		return -1;
	}

	/**********lock************/
	if(sem_wait(&this->lock)==-1){
		return -1;
	}
	/**************************/

	if(this->head==NULL){
		return -1;
	}
	*size = this->head->size;

	*args = this->head->data;

	this->head = this->head->next;

	/**********unlock************/
	if(sem_post(&this->lock)==-1){
		return -1;
	}
	/**************************/
	
	return 0;
}


int fifo_size(Fifo* this){
	int sval = 0;
	sem_getvalue(&this->size,&sval);
	return sval;		
}


