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
	
queue_t ready_queue;

static void init_pcb()
{
/* 	// Initialize each pcb
	int i;
	for (i = 0; i < NUM_MAX_TASK; i++) 
		pcb[i].status = TASK_EXITED;
*/
	int i, j;
	queue_init(&ready_queue);

	// kernel pcb 
	pcb_t kn_pcb;
	kn_pcb.pid = 1;
	kn_pcb.type = KERNEL_THREAD;
	kn_pcb.parent = NULL;
	kn_pcb.status = TASK_RUNNING;
	current_running = &kn_pcb;

	// Initialize the sched1_tasks pcb 
	for (i = 0; i < num_sched1_tasks; i++) {
		pcb[i].pid = i + 10;
		pcb[i].type = sched1_tasks[i]->type;
		pcb[i].kernel_stack_top = (uint32_t)(STACK_TOP - i * KN_STACK_SIZE);
		pcb[i].parent = NULL;
		pcb[i].kernel_context.regs[0] = 1;
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)sched1_tasks[i]->entry_point;
		pcb[i].status = TASK_READY;
		queue_push(&ready_queue, &pcb[i]);
	}


	// lock1_task
	for (j = 0; j < num_lock_tasks; i++, j++) {
		pcb[i].pid = i + 10;
		pcb[i].type = lock_tasks[j]->type;
		pcb[i].kernel_stack_top = (uint32_t)(STACK_TOP - i * KN_STACK_SIZE);
		pcb[i].parent = NULL;
		pcb[i].kernel_context.regs[0] = 1;
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)lock_tasks[j]->entry_point;
		pcb[i].status = TASK_READY;
		queue_push(&ready_queue, &pcb[i]);
	}

	return;
}

static void init_exception_handler()
{
}

static void init_exception()
{
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
	// 3. Copy the level 2 exception handling code to 0x80000180
	// 4. reset CP0_COMPARE & CP0_COUNT register
}

static void init_syscall(void)
{
	// init system call table.
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
	
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		do_scheduler();
	};
	return;
}
