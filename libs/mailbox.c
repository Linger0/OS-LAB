#include "string.h"
#include "mailbox.h"
#include "lock.h"
#include "syscall.h"

#define MAX_NUM_BOX 32

static mailbox_t mboxs[MAX_NUM_BOX] = {0};
static mutex_lock_t lock;

void mbox_init(mailbox_t *mailbox)
{	// 初始化指定邮箱
	mailbox->open = 0;
	mailbox->len = 0;
	mailbox->si = mailbox->ri = 0;
	condition_init(&mailbox->send);
	condition_init(&mailbox->recv);
	mutex_lock_init(&mailbox->mutex);
}

mailbox_t *mbox_open(char *name)
{
	int i;
	mutex_lock_acquire(&lock);
	for (i = 0; i < MAX_NUM_BOX; i++) { // 寻找邮箱
		if (mboxs[i].open > 0 && !strcmp(mboxs[i].name, name)) break;
	};
	if (i == MAX_NUM_BOX) { // 未找到指定邮箱
		for (i = 0; i < MAX_NUM_BOX && mboxs[i].open; i++);
		mboxs[i].name = name;
		mbox_init(&mboxs[i]);
	}
	mboxs[i].open++;
	mutex_lock_release(&lock);
	return &mboxs[i];
}

void mbox_close(mailbox_t *mailbox)
{
	mutex_lock_acquire(&lock);
	mailbox->open--;
	mutex_lock_release(&lock);
}

void mbox_send(mailbox_t *mailbox, void *msg, int msg_length)
{
	char *_msg = msg;
	mutex_lock_acquire(&mailbox->mutex);
	for (msg_length; msg_length > 0; msg_length--) {
		while (mailbox->len == MAX_LEN_MSG) {
			condition_wait(&mailbox->mutex, &mailbox->send);
		}
		mailbox->msg[mailbox->si] = *(_msg++);
		mailbox->len++;
		mailbox->si = (mailbox->si + 1) % MAX_LEN_MSG;
		if (mailbox->len == 1) {
			condition_broadcast(&mailbox->recv);
		}
	}
	mutex_lock_release(&mailbox->mutex);
}

void mbox_recv(mailbox_t *mailbox, void *msg, int msg_length)
{
	char *_msg = msg;
	mutex_lock_acquire(&mailbox->mutex);
	for (msg_length; msg_length > 0; msg_length--) {
		while (mailbox->len == 0) {
			condition_wait(&mailbox->mutex, &mailbox->recv);
		}
		*(_msg++) = mailbox->msg[mailbox->ri];
		mailbox->len--;
		mailbox->ri = (mailbox->ri + 1) % MAX_LEN_MSG;
		if (mailbox->len == MAX_LEN_MSG - 1) {
			condition_broadcast(&mailbox->send);
		}
	}
	mutex_lock_release(&mailbox->mutex);
}