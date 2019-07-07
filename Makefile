LDLIBS += -lpthread
CFLAGS += -g

all: smq_test

smq_test: smq.c
