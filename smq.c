#include "smq.h"
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

struct smq {
	size_t head;
	size_t tail;
	size_t size;
	sem_t used;
	sem_t free;
	smq_msg_t msgs[];
};

smq_t* smq_open(const char* name) {
	size_t size = 512;
	smq_t* smq = malloc(sizeof(smq_t)+sizeof(smq_msg_t)*size);
	smq->head = 0;
	smq->tail = 0;
	smq->size = size;
	sem_init(&smq->used,0,0);
	sem_init(&smq->free,0,size);
	for(size_t i=0; i<size; i++) {
		smq_msg_init(smq->msgs+i);
	}
	return smq;
}

void smq_close(smq_t* smq) {
	sem_destroy(&smq->used);
	sem_destroy(&smq->free);
	free(smq);
}
void smq_send(smq_t* smq,smq_msg_t* src) {
	if(sem_wait(&smq->free)==0) {
		size_t tail = smq->tail;
		while(__sync_val_compare_and_swap(&smq->tail,tail,(tail + 1) % smq->size)!=tail) tail = smq->tail;
		smq_msg_move(&smq->msgs[tail],src);
		sem_post(&smq->used);
	}
}

void smq_receive(smq_t* smq,smq_msg_t* dest) {
	if(sem_wait(&smq->used)==0) {
		size_t head = smq->head;
		while(__sync_val_compare_and_swap(&smq->head,head,(head + 1) % smq->size)!=head) head = smq->head;
		smq_msg_move(dest,&smq->msgs[head]);
		sem_post(&smq->free);
	}
}

void smq_msg_init(smq_msg_t* msg) {
	msg->allocated = 0;
	msg->size = 0;
	msg->data = 0;
}

void smq_msg_write(smq_msg_t* msg,const void* data,size_t size) {
	if(msg->allocated < msg->size + size) {
		if(msg->allocated == 0) msg->allocated = 8;
		while(msg->allocated < msg->size + size) msg->allocated *= 2;
		msg->data = realloc(msg->data,msg->allocated);
	}
	memcpy(msg->data+msg->size,data,size);
}

void smq_msg_release(smq_msg_t* msg) {
	free(msg->data);
	smq_msg_init(msg);
}

void smq_msg_move(smq_msg_t* dest,smq_msg_t* src) {
	*dest = *src;
	smq_msg_init(src);
}

const void* smq_msg_data(smq_msg_t* msg) {
	return msg->data;
}

size_t smq_msg_size(smq_msg_t* msg) {
	return msg->size;
}
