#include "irq.h"
#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"
// #include "mm.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;

/* ready/block/wait queue */
queue_t ready_queue;
queue_t sleep_queue;
queue_t wait_queue;

static void check_sleeping()
{
	pcb_t *sleep_pcb, *nxt;
	uint32_t cur_time = get_timer();
	sleep_pcb = sleep_queue.head;
	while (sleep_pcb) {
		if (cur_time > sleep_pcb->wakeup_time) {
			nxt = queue_remove(&sleep_queue, sleep_pcb);
			sleep_pcb->status = TASK_READY;
			queue_push(&ready_queue, sleep_pcb);
			sleep_pcb = nxt;
		}
		else {
			sleep_pcb = sleep_pcb->next;
		}
	}
}

void scheduler(void)
{	
	// Modify the current_running pointer.

	if (current_running->status == TASK_RUNNING) {
		current_running->status = TASK_READY;
		queue_push(&ready_queue, current_running);
	}

	current_running->cursor_x = screen_cursor_x;
	current_running->cursor_y = screen_cursor_y;

	current_running = queue_dequeue(&ready_queue);
	if (current_running->status == TASK_EXITED) do_exit(); // 处理僵尸进程
	current_running->status = TASK_RUNNING;

	screen_cursor_x = current_running->cursor_x;
	screen_cursor_y = current_running->cursor_y;

	if (rst_timer) {
		get_ticks(); // 更新 time_elapsed
		reset_cp0_count(); 
		set_cp0_compare(TIMER_INTERVAL); // 重置时间片
		rst_timer = 0; 
		if (!queue_is_empty(&sleep_queue)) check_sleeping(); // check sleeping
	} 

/*	// 设置ASID
	uint32_t cp0_entryhi = get_cp0_entryhi();
	cp0_entryhi &= ~0xfff;
	cp0_entryhi |= current_running->pid;
	set_cp0_entryhi(cp0_entryhi);
*/
}

void do_sleep(uint32_t sleep_time)
{
	current_running->wakeup_time = get_timer() + sleep_time;
	current_running->status = TASK_BLOCKED;
	queue_push(&sleep_queue, current_running);
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
	pcb_t *unblock_pcb = queue_dequeue(queue);
	unblock_pcb->status = TASK_READY;
	queue_push(&ready_queue, unblock_pcb);
}

void do_unblock_all(queue_t *queue)
{
	// unblock all task in the queue
	pcb_t *unblock_pcb;
	while (!queue_is_empty(queue)) {
		unblock_pcb = queue_dequeue(queue);
		unblock_pcb->status = TASK_READY;
		queue_push(&ready_queue, unblock_pcb);
	}
}

void do_spawn(task_info_t *task) 
{
	int i, k;
	for (i = 0; i < NUM_MAX_TASK && pcb[i].status != TASK_EXITED; i++);
	if (i == NUM_MAX_TASK) return;

	bzero(&pcb[i], 2 * sizeof(regs_context_t));
	pcb[i].pid = process_id++;
	pcb[i].type = task->type;
	pcb[i].status = TASK_READY;
	queue_init(&pcb[i].lock_queue);
	pcb[i].user_context.regs[29] = pcb[i].user_stack_top;
	pcb[i].kernel_context.regs[31] = (uint32_t)new_proc_run;
	pcb[i].kernel_context.cp0_status =
	pcb[i].user_context.cp0_status = initial_cp0_status;
	pcb[i].user_context.cp0_epc = task->entry_point;
	pcb[i].cwd = current_running->cwd;

/*	// 初始化页目录
	pde_t *pde = (pde_t *)pcb[i].pde_base;
	for (k = 0; k < 1024; k++) {
		pde[k] = initial_pde;
	}
*/
	queue_push(&ready_queue, &pcb[i]);
}

void do_kill(pid_t pid) 
{
	if (pid == current_running->pid) do_exit(); // 杀死自己

	int i;
	for (i = 0; i < NUM_MAX_TASK && pcb[i].pid != pid; i++);
	if (i == NUM_MAX_TASK || pcb[i].status == TASK_EXITED) return;	

	if (pcb[i].status == TASK_BLOCKED) {
		queue_remove(pcb[i].belong, &pcb[i]);
		queue_push(&ready_queue, &pcb[i]);
	}
	pcb[i].status = TASK_EXITED;  // 成为 ready_queue 的僵尸进程
}

void do_exit(void) 
{
	pcb_t *wait_pcb, *nxt;
	mutex_lock_t *pcb_lock;
	while (!queue_is_empty(&current_running->lock_queue)) { // release mutex_lock
		pcb_lock = (current_running->lock_queue).head;
		do_mutex_lock_release(pcb_lock);
	}
	wait_pcb = wait_queue.head;
	while (wait_pcb) { // unblock wait pcb
		if (wait_pcb->wait_pid == current_running->pid) {
			nxt = queue_remove(&wait_queue, wait_pcb);
			wait_pcb->status = TASK_READY;
			queue_push(&ready_queue, wait_pcb);
			wait_pcb = nxt;
		}
		else {
			wait_pcb = wait_pcb->next;
		}
	}
	current_running->pid = 0; // 清除 pid
	current_running->status = TASK_EXITED;
	do_scheduler();
}

void do_waitpid(pid_t pid) 
{
	current_running->wait_pid = pid;
	current_running->status = TASK_BLOCKED;
	queue_push(&wait_queue, current_running);
	do_scheduler();
}

int do_getpid(void)
{
	return current_running->pid;
}