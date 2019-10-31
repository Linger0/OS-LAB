#include "barrier.h"
#include "sched.h"

void do_barrier_init(barrier_t *barrier, int goal)
{
	barrier->n = 0;
	barrier->goal = goal;
	queue_init(&barrier->block_queue);
}

void do_barrier_wait(barrier_t *barrier)
{
	barrier->n++;
	if (barrier->n == barrier->goal) {
		do_unblock_all(&barrier->block_queue);
		barrier->n = 0;
	} else {
		do_block(&barrier->block_queue);
	}
}