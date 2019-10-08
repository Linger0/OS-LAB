#include "lock.h"
#include "sched.h"
#include "syscall.h"

void spin_lock_init(spin_lock_t *lock)
{
	lock->status = UNLOCKED;
}

void spin_lock_acquire(spin_lock_t *lock)
{
	while (LOCKED == lock->status)
		do_scheduler();
	lock->status = LOCKED;
}

void spin_lock_release(spin_lock_t *lock)
{
	lock->status = UNLOCKED;
}

void do_mutex_lock_init(mutex_lock_t *lock)
{
	int i;
	for (i = 0; i < NUM_LOCK; i++) {
		lock[i].status = lock_max_thread[i];
		queue_init(&lock[i].lock_queue);
	}
}

void do_mutex_lock_acquire(mutex_lock_t *lock)
{
	if (lock->status == LOCKED)
		do_block(&lock->lock_queue);
	else
		lock->status--;
}

void do_mutex_lock_release(mutex_lock_t *lock)
{
	if (!queue_is_empty(&lock->lock_queue))
		do_unblock_one(&lock->lock_queue);
	else
		lock->status++;
}
