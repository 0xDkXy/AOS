#ifndef __DEVICE_IOQUEUE_H
#define __DEVICE_IOQUEUE_H
#include "stdint.h"
#include "thread.h"
#include "sync.h"

#define BUFSIZE 64

struct ioqueue {
    struct lock lock;
    struct task_strcut* producer;

    struct task_struct* consumer;
    char buf[BUFSIZE];
    int32_t head;
    int32_t tail;
};

void ioqueue_init(struct ioqueue* ioq);
char ioq_getchar(struct ioqueue* ioq);
bool ioq_empty(struct ioqueue* ioq);
bool ioq_full(struct ioqueue* ioq);
void ioq_putchar(struct ioqueue* ioq, char byte);

#endif // __DEVICE_IOQUEUE_H
