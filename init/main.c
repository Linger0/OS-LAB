/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"

uint32_t initial_priority[] = {0, 8, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
uint32_t initial_cp0_status = 0x10008003;

pcb_t pcbk;
void (*exception_handler[NUM_EXCCODE])();
int (*syscall[NUM_SYSCALLS])();

static void init_pcb()
{
	int i = 0, j = 0;
	uint32_t initial_stack = STACK_BASE;
	queue_init(&ready_queue);
	queue_init(&block_queue);

	// kernel pcb 
	pcbk.pid = process_id++;
	pcbk.type = KERNEL_THREAD;
	pcbk.status = TASK_RUNNING;
	pcbk.priority = initial_priority[0]; // 内核线程优先级最小
	pcbk.kernel_stack_top = 
	pcbk.kernel_context.regs[29] = initial_stack;

	initial_stack -= STACK_SIZE;
	current_running = &pcbk;

	// Initialize the sched1_tasks pcb 
	for (i = 0; i < num_sched2_tasks; i++) {
		bzero(&pcb[i].kernel_context,sizeof(regs_context_t));
		bzero(&pcb[i].user_context,sizeof(regs_context_t));

		pcb[i].pid = process_id++;
		pcb[i].type = sched1_tasks[i]->type;
		pcb[i].status = TASK_READY;
		pcb[i].priority = initial_priority[i + 1];

		pcb[i].kernel_stack_top = initial_stack; initial_stack -= STACK_SIZE;
		pcb[i].user_stack_top = initial_stack; initial_stack -= STACK_SIZE;
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)new_proc_run; // 新进程入口, do_scheduler 跳转
		pcb[i].kernel_context.cp0_status = 
		pcb[i].user_context.cp0_status = initial_cp0_status;
		pcb[i].kernel_context.cp0_epc = 
		pcb[i].user_context.cp0_epc = sched2_tasks[i]->entry_point;

		queue_push(&ready_queue, &pcb[i]);
	}

	for (j = 0; j < num_lock_tasks; i++, j++) {
		bzero(&pcb[i].kernel_context,sizeof(regs_context_t));
		bzero(&pcb[i].user_context,sizeof(regs_context_t));

		pcb[i].pid = process_id++;
		pcb[i].type = lock_tasks[j]->type;
		pcb[i].status = TASK_READY;
		pcb[i].priority = initial_priority[i + 1];

		pcb[i].kernel_stack_top = initial_stack; initial_stack -= STACK_SIZE;
		pcb[i].user_stack_top = initial_stack; initial_stack -= STACK_SIZE;
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)new_proc_run; // 新进程入口, do_scheduler 跳转
		pcb[i].kernel_context.cp0_status = 
		pcb[i].user_context.cp0_status = initial_cp0_status;
		pcb[i].kernel_context.cp0_epc = 
		pcb[i].user_context.cp0_epc = lock_tasks[j]->entry_point;

		queue_push(&ready_queue, &pcb[i]);
	}

	for (j = 0; j < num_timer_tasks; i++, j++) {
		bzero(&pcb[i].kernel_context,sizeof(regs_context_t));
		bzero(&pcb[i].user_context,sizeof(regs_context_t));

		pcb[i].pid = process_id++;
		pcb[i].type = timer_tasks[j]->type;
		pcb[i].status = TASK_READY;
		pcb[i].priority = initial_priority[i + 1];

		pcb[i].kernel_stack_top = initial_stack; initial_stack -= STACK_SIZE;
		pcb[i].user_stack_top = initial_stack; initial_stack -= STACK_SIZE;
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)new_proc_run; // 新进程入口, do_scheduler 跳转
		pcb[i].kernel_context.cp0_status = 
		pcb[i].user_context.cp0_status = initial_cp0_status;
		pcb[i].kernel_context.cp0_epc = 
		pcb[i].user_context.cp0_epc = timer_tasks[j]->entry_point;

		queue_push(&ready_queue, &pcb[i]);
	}
}

static void init_exception_handler()
{
	int i;
	for (i = 0; i < NUM_EXCCODE; i++) {
		exception_handler[i] = (void (*)())handle_other;
	}
	exception_handler[INT] = (void (*)())handle_int;
	exception_handler[SYS] = (void (*)())handle_syscall;
}

static void init_exception()
{
	init_cp0_status(initial_cp0_status);
	init_exception_handler();
	memcpy(	(uint8_t *)(BEV0_EBASE + BEV0_OFFSET), 
		(uint8_t *)exception_handler_entry, 
		(uint32_t)(exception_handler_end - exception_handler_begin) );
	reset_cp0_count(); set_cp0_compare(TIMER_INTERVAL);
}

static void init_syscall(void)
{
	// init system call table.
	syscall[SYSCALL_SLEEP] = (int(*)())do_sleep;
	syscall[SYSCALL_BLOCK] = (int (*)())do_block;
	syscall[SYSCALL_UNBLOCK_ONE] = (int (*)())do_unblock_one;
	syscall[SYSCALL_UNBLOCK_ALL] = (int (*)())do_unblock_all;
	syscall[SYSCALL_WRITE] = (int (*)())screen_write;
/* 	syscall[SYSCALL_READ] = (int (*)()) ;		*/
	syscall[SYSCALL_CURSOR] = (int (*)())screen_move_cursor;
	syscall[SYSCALL_REFLUSH] = (int (*)())screen_reflush;
	syscall[SYSCALL_MUTEX_LOCK_INIT] = (int (*)())do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (int (*)())do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (int (*)())do_mutex_lock_release;
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	// Close the cache, no longer refresh the cache 
	// when making the exception vector entry copy
	asm_start();

	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");

	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	printk("> [INIT] SCREEN initialization succeeded.\n");

	// TODO Enable interrupt
	init_enable_int();

	while (1)
	{
	};
	return;
}
