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
#include "mailbox.h"
// #include "mm.h"
#include "mac.h"

uint32_t initial_cp0_status = 0x30008003;

void (*exception_handler[32])();
int (*syscall[NUM_SYSCALLS])();
/*
static void init_memory()
{
	init_page_table(); 	// 初始化页目录与一页页表
	init_swap();
	//init_TLB();
}
*/
static void init_pcb()
{
	int i;
	for (i = 0; i < NUM_MAX_TASK; i++) {
		pcb[i].pid = 0;
		pcb[i].status = TASK_EXITED;
		pcb[i].kernel_stack_top = STACK_BASE - 4 + (2 * i + 1) * STACK_SIZE;
		pcb[i].user_stack_top = STACK_BASE - 4 + (2 * i + 2) * STACK_SIZE;
		// pcb[i].pde_base = PGDIR_BASE + (i + 1) * PAGE_SIZE;
	}
	queue_init(&ready_queue);
	queue_init(&sleep_queue);
	queue_init(&wait_queue);

	bzero(&pcb[0], 2 * sizeof(regs_context_t));
	pcb[0].pid = process_id++;
	pcb[0].type = USER_PROCESS;
	pcb[0].status = TASK_READY;
	queue_init(&pcb[0].lock_queue);
	pcb[0].user_context.regs[29] = pcb[0].user_stack_top;
	pcb[0].kernel_context.regs[31] = (uint32_t)new_proc_run;
	pcb[0].kernel_context.cp0_status =
	pcb[0].user_context.cp0_status = initial_cp0_status;
	pcb[0].user_context.cp0_epc = (uint32_t)test_shell;
	queue_push(&ready_queue, &pcb[0]);

	// for first schedule
	current_running = &pcb[1];
}

static void init_exception_handler()
{
	int i;
	for (i = 0; i < 32; i++) {
		exception_handler[i] = (void (*)())handle_other;
	}
	// exception_handler[TLBL] = (void (*)())handle_TLBinvalid;
	// exception_handler[TLBS] = (void (*)())handle_TLBinvalid;
	exception_handler[INT] = (void (*)())handle_int;
	exception_handler[SYS] = (void (*)())handle_syscall;
}

static void init_exception()
{
	uint32_t cp0_status = initial_cp0_status;
	cp0_status &= ~0x3;
	set_cp0_status(cp0_status);

	init_exception_handler();

	memcpy(	(uint8_t *)(BEV0_EBASE + BEV0_OFFSET), 
		(uint8_t *)exception_handler_entry, 
		(uint32_t)(exception_handler_end - exception_handler_begin) );
/*
	memcpy(	(uint8_t *)BEV0_EBASE, 
		(uint8_t *)TLBexception_handler_entry, 
		(uint32_t)(TLBexception_handler_end - TLBexception_handler_begin) );
*/	
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
// 	syscall[SYSCALL_READ] = (int (*)()) ;		
	syscall[SYSCALL_CURSOR] = (int (*)())screen_move_cursor;
	syscall[SYSCALL_REFLUSH] = (int (*)())screen_reflush;
	syscall[SYSCALL_CLEAR] = (int (*)())screen_clear;
	syscall[SYSCALL_GETPID] = (int (*)())do_getpid;
	syscall[SYSCALL_MUTEX_LOCK_INIT] = (int (*)())do_mutex_lock_init;
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (int (*)())do_mutex_lock_acquire;
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (int (*)())do_mutex_lock_release;
	syscall[SYSCALL_SPAWN] = (int (*)())do_spawn;
	syscall[SYSCALL_EXIT] = (int (*)())do_exit;
	syscall[SYSCALL_KILL] = (int (*)())do_kill;
	syscall[SYSCALL_WAIT] = (int (*)())do_waitpid;
	syscall[SYSCALL_SEMAPHORE_INIT] = (int (*)())do_semaphore_init;
	syscall[SYSCALL_SEMAPHORE_UP] = (int (*)())do_semaphore_up;
	syscall[SYSCALL_SEMAPHORE_DOWN] = (int (*)())do_semaphore_down;
	syscall[SYSCALL_CONDITION_INIT] = (int (*)())do_condition_init;
	syscall[SYSCALL_CONDITION_WAIT] = (int (*)())do_condition_wait;
	syscall[SYSCALL_CONDITION_SIGNAL] = (int (*)())do_condition_signal;
	syscall[SYSCALL_CONDITION_BROADCAST] = (int (*)())do_condition_broadcast;
	syscall[SYSCALL_BARRIER_INIT] = (int (*)())do_barrier_init;
	syscall[SYSCALL_BARRIER_WAIT] = (int (*)())do_barrier_wait;
	syscall[SYSCALL_INIT_MAC] = (int (*)())do_init_mac;
	syscall[SYSCALL_NET_SEND] = (int (*)())do_net_send;
	syscall[SYSCALL_NET_RECV] = (int (*)())do_net_recv;
	syscall[SYSCALL_WAIT_RECV_PACKAGE] = (int (*)())do_wait_recv_package;
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
/*
	// init virtual memory
	init_memory();
	printk("> [INIT] Virtual memory initialization succeeded.\n");
*/
	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();
	// printk("> [INIT] SCREEN initialization succeeded.\n");

	// Enable interrupt
	uint32_t cp0_status = get_cp0_status();
	cp0_status |= 0x1;
	set_cp0_status(cp0_status);

	while (1)
	{
	};
}
