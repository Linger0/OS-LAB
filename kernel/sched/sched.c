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
			_item->priority = (_item->priority << 4) | 0b1111; // 被阻塞任务唤醒后优先级提升
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

	// 下降 current_running->priority, 若初始非0则始终非0, 初始为0则始终为0
	if (current_running->priority)
		current_running->priority = (current_running->priority >> 3) | 0b1;

	get_ticks(); // 更新 time_elapsed
	reset_cp0_count();

	if (!queue_is_empty(&block_queue)) check_sleeping(); // check sleeping

	// 调用优先级最大的进程, 在相同优先级下满足先进先出
	current_running = ready_queue.head;
	for (_item = ready_queue.head; _item; _item = _item->next)
		if (_item->priority > current_running->priority)
			current_running = _item;
	queue_remove(&ready_queue, current_running);
	current_running->status = TASK_RUNNING;
	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;

	// 增加未调用进程的 priority, 以 (00..011..1) 的形式呈现
	for (_item = ready_queue.head; _item; _item = _item->next) 
		if (_item->priority) _item->priority = (_item->priority << 1) | 0b1;

	get_ticks(); // 更新 time_elapsed
	reset_cp0_count(); set_cp0_compare(TIMER_INTERVAL); // 重置时间片
}

void do_sleep(uint32_t sleep_time)
{
	get_ticks(); reset_cp0_count();
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
	unblock_proc->priority = (unblock_proc->priority << 4) | 0b1111; // 被阻塞任务唤醒后优先级提升
	queue_push(&ready_queue, unblock_proc);
}

void do_unblock_all(queue_t *queue)
{
	// unblock all task in the queue
}
