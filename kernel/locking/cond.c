#include "cond.h"
#include "lock.h"
#include "sched.h"

void do_condition_init(condition_t *condition)
{
	queue_init(&condition->block_queue);
}

void do_condition_wait(mutex_lock_t *lock, condition_t *condition)
{
	do_mutex_lock_release(lock);
	do_block(&condition->block_queue);
	do_mutex_lock_acquire(lock);
}

void do_condition_signal(condition_t *condition)
{
	if (!queue_is_empty(&condition->block_queue)) {
		do_unblock_one(&condition->block_queue);
	}
}

void do_condition_broadcast(condition_t *condition)
{
	do_unblock_all(&condition->block_queue);
}