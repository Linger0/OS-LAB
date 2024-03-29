#include "lock.h"
#include "sched.h"
#include "syscall.h"

void spin_lock_init(spin_lock_t *lock)
{
	lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
	while (LOCKED == lock->status);
	lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
	lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
	lock->status = UNLOCKED;
	queue_init(&lock->block_queue);
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
	if (lock->status == LOCKED) {
		do_block(&lock->block_queue);
	} else {
		lock->status = LOCKED;
	}	
	lqueue_push(&current_running->lock_queue, lock);
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
	lqueue_remove(&current_running->lock_queue, lock);
	if (!queue_is_empty(&lock->block_queue)) {
		do_unblock_one(&lock->block_queue);
	} else {
		lock->status = UNLOCKED;
	}
}
