#include <stddef.h>

typedef struct smq_msg smq_msg_t;
typedef struct smq smq_t;

smq_t* smq_open(const char* name);
void smq_close(smq_t* smq);
void smq_send(smq_t* smq,smq_msg_t* msg);
void smq_receive(smq_t* smq,smq_msg_t* msg);

void smq_msg_init(smq_msg_t* msg);
void smq_msg_write(smq_msg_t* msg,const void* data,size_t size);
void smq_msg_release(smq_msg_t* msg);
void smq_msg_copy(smq_msg_t* dest,smq_msg_t* src);
void smq_msg_move(smq_msg_t* dest,smq_msg_t* src);
const void* smq_msg_data(smq_msg_t* msg);
size_t smq_msg_size(smq_msg_t* msg);

struct smq_msg {
	size_t allocated;
	size_t size;
	void* data;
};
