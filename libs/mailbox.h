#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include "cond.h"
#include "syscall.h"

#define MAX_LEN_MSG 12

typedef struct mailbox
{
    char *name;
    int open;
    char msg[MAX_LEN_MSG];
    int len;
    int si;
    int ri;
    condition_t send;
    condition_t recv;
    mutex_lock_t mutex;
} mailbox_t;


void mbox_init(mailbox_t *);
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif