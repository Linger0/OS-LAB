#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;

/* ready queue */
queue_t ready_queue;

static void check_sleeping()
{
}

void scheduler(void)
{
	// Modify the current_running pointer.
	if (current_running->status == TASK_BLOCKED) 
		queue_push(&block_queue, current_running);
	else {
		current_running->status = TASK_READY;
		queue_push(&ready_queue, current_running);
	}
	current_running = queue_dequeue(&ready_queue);
	process_id = current_running->pid;
	current_running->status = TASK_RUNNING;
}

void do_sleep(uint32_t sleep_time)
{
	// TODO sleep(seconds)
}

void do_block(queue_t *queue)
{
    	// block the current_running task into the queue
	current_running->status = TASK_BLOCKED;
	do_scheduler();
}

void do_unblock_one(queue_t *queue)
{
	// unblock the head task from the queue
	if (queue_is_empty(queue)) return;
	else {
		pcb_t *unblock_process;
		unblock_process = queue_dequeue(queue);
		unblock_process->status = TASK_READY;
		queue_push(&ready_queue, unblock_process);
	}
}

void do_unblock_all(queue_t *queue)
{
	// unblock all task in the queue
}
