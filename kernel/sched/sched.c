#include "irq.h"
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

/* ready/block queue */
queue_t ready_queue;
queue_t block_queue;

static void check_sleeping()
{
	pcb_t *_item;
	uint32_t cur_time = get_timer();
	for (_item = block_queue.head; _item; _item = _item->next)
		if (cur_time > _item->wakeup_time) {
			queue_remove(&block_queue, _item);
			_item->status = TASK_READY;
			queue_push(&ready_queue, _item);
		}
}

void scheduler(void)
{	
	// Modify the current_running pointer.

	pcb_t *_item;
	if (current_running->status == TASK_RUNNING) {
		current_running->status = TASK_READY;
		queue_push(&ready_queue, current_running);
	}
	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;

	// 重置priority
	if (current_running->priority)
		current_running->priority = initial_priority;

	// 调用优先级最大的进程, 在相同优先级下满足先进先出
	current_running = ready_queue.head;
	for (_item = ready_queue.head; _item; _item = _item->next)
		if (_item->priority > current_running->priority)
			current_running = _item;
	queue_remove(&ready_queue, current_running);
	current_running->status = TASK_RUNNING;
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;

	// 增加未调用进程的 priority
	for (_item = ready_queue.head; _item; _item = _item->next) 
		if (_item->priority) _item->priority++;

	if (rst_timer) {
		get_ticks(); // 更新 time_elapsed
		reset_cp0_count(); set_cp0_compare(TIMER_INTERVAL); // 重置时间片
		rst_timer = 0;
		if (!queue_is_empty(&block_queue)) check_sleeping(); // check sleeping
	}
}

void do_sleep(uint32_t sleep_time)
{
	current_running->wakeup_time = get_timer() + sleep_time;
	current_running->status = TASK_BLOCKED;
	queue_push(&block_queue, current_running);
	do_scheduler();
}

void do_block(queue_t *queue)
{
    // block the current_running task into the queue
	current_running->status = TASK_BLOCKED;
	queue_push(queue, current_running);
	do_scheduler();
}

void do_unblock_one(queue_t *queue)
{
	// unblock the head task from the queue
	pcb_t *unblock_proc;
	unblock_proc = queue_dequeue(queue);
	unblock_proc->status = TASK_READY;
	queue_push(&ready_queue, unblock_proc);
}

void do_unblock_all(queue_t *queue)
{
	// unblock all task in the queue
}
