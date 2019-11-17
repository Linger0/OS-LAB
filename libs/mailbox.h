#ifndef INCLUDE_MAIL_BOX_
#define INCLUDE_MAIL_BOX_

#include "cond.h"
#include "syscall.h"

#define MAX_LEN_MSG 12

typedef struct mailbox
{
    char *name; // 信箱名(open与close使用)
    int open;   // 引用数
    char msg[MAX_LEN_MSG];  // 消息
    int len;    // 当前有效消息长度
    int si;     // send索引
    int ri;     // recv索引
    condition_t send;   // 信箱满时阻塞发送方
    condition_t recv;   // 信箱空时阻塞接收方
    mutex_lock_t mutex; // 信箱访问互斥
} mailbox_t;


void mbox_init(mailbox_t *);
mailbox_t *mbox_open(char *);
void mbox_close(mailbox_t *);
void mbox_send(mailbox_t *, void *, int);
void mbox_recv(mailbox_t *, void *, int);

#endif