#include "smq.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 200

typedef struct {
	pthread_t thread;
	unsigned numthread;
	unsigned nbmsgs;
	smq_t* smq;
} thread_params_t;

void* send(void* arg) {
	thread_params_t* params = (thread_params_t*) arg;
	char* data = malloc(BUFFER_SIZE);
	for(unsigned i=0; i<params->nbmsgs; i++) {
		int len = snprintf(data,BUFFER_SIZE,"Message%d from worker %d",i,params->numthread);
		smq_msg_t msg;
		smq_msg_init(&msg);
		smq_msg_write(&msg,data,len+1);
		smq_send(params->smq,&msg);
	}
	free(data);
	return 0;
}

void* receive(void* arg) {
	thread_params_t* params = (thread_params_t*) arg;
	for(unsigned i=0; i<params->nbmsgs; i++) {
		smq_msg_t msg;
		//smq_msg_init(&msg);
		smq_receive(params->smq,&msg);
		printf("worker %d received %s\n",params->numthread,(char*) smq_msg_data(&msg));
		smq_msg_release(&msg);
	}
	return 0;
}

int test(unsigned nbmsgs,unsigned nbsenders,unsigned nbreceivers) {
	unsigned nbthreads = nbsenders + nbreceivers;
	thread_params_t* params = malloc(sizeof(thread_params_t)*nbthreads);
	smq_t* smq = smq_open("");
	for(unsigned i=0; i<nbsenders; i++) {
		params[i].numthread = i;
		params[i].nbmsgs = nbmsgs / nbsenders;
		if(i==0) params[i].nbmsgs += nbmsgs % nbsenders;
		params[i].smq = smq;
		pthread_create(&params[i].thread,0,send,&params[i]);
	}
	for(unsigned i=nbsenders; i<nbthreads; i++) {
		params[i].numthread = i;
		params[i].nbmsgs = nbmsgs / nbreceivers;
		if(i==nbsenders) params[i].nbmsgs += nbmsgs % nbreceivers;
		params[i].smq = smq;
		pthread_create(&params[i].thread,0,receive,&params[i]);
	}
	for(unsigned i=0; i<nbthreads; i++) {
		void* retval;
		pthread_join(params[i].thread,&retval);
	}
	smq_close(smq);
	free(params);
	return 0;
}

int main(int argc,char** argv) {
	unsigned nbmsgs = 1;
	unsigned nbsenders = 1;
	unsigned nbreceivers = 1;
	if(argc>1) nbmsgs = atoi(argv[1]);
	if(argc>2) nbsenders = atoi(argv[2]);
	if(argc>3) nbreceivers = atoi(argv[3]);
	return test(nbmsgs,nbsenders,nbreceivers);
}
