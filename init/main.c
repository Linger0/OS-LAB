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

pcb_t kn_pcb;
uint32_t initial_cp0_status = 0x10008001;
uint32_t initial_priority = 10;
uint32_t exception_handler[32];

static void init_pcb()
{
	int i, j;
	uint32_t stack_top = STACK_BASE;
	queue_init(&ready_queue);

	// kernel pcb 
	kn_pcb.pid = process_id++;
	kn_pcb.status = TASK_RUNNING;
	kn_pcb.priority = kn_pcb.runtime = 0; // 内核线程优先级最小
	current_running = &kn_pcb;

	// Initialize the sched1_tasks pcb 
	for (i = 0; i < num_sched1_tasks; i++) {
		bzero(&pcb[i].kernel_context,sizeof(regs_context_t));
		bzero(&pcb[i].user_context,sizeof(regs_context_t));

		pcb[i].pid = process_id++;
		pcb[i].type = sched1_tasks[i]->type;
		pcb[i].status = TASK_READY;
		pcb[i].runtime = 0; pcb[i].priority = initial_priority;

		pcb[i].kernel_stack_top = (uint32_t)stack_top; stack_top -= STACK_SIZE;
		pcb[i].user_stack_top = (uint32_t)stack_top; stack_top -= STACK_SIZE;
		pcb[i].kernel_context.regs[0] = pcb[i].user_context.regs[0] = 1; /* new process */
		pcb[i].kernel_context.regs[29] = pcb[i].kernel_stack_top;
		pcb[i].user_context.regs[29] = pcb[i].user_stack_top;
		pcb[i].kernel_context.regs[31] = (uint32_t)&new_proc_run;
		pcb[i].user_context.regs[31] = (uint32_t)sched1_tasks[i]->entry_point;
		pcb[i].kernel_context.cp0_epc = pcb[i].user_context.cp0_epc = 
			(uint32_t)sched1_tasks[i]->entry_point;

		queue_push(&ready_queue, &pcb[i]);
	}
}

static void init_exception_handler()
{
	int i;
	exception_handler[0] = (uint32_t)&handle_int;
	for (i = 1; i < 32; i++)
		exception_handler[i] = (uint32_t)&handle_other;
	exception_handler[8] = (uint32_t)&handle_syscall;
}

static void init_exception()
{
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
	// 3. Copy the level 2 exception handling code to 0x80000180
	// 4. reset CP0_COMPARE & CP0_COUNT register
	init_status(initial_cp0_status);
	init_exception_handler();
	memcpy((uint8_t *)(BEV0_EBASE + BEV0_OFFSET), (uint8_t *)exception_handler_entry, 
		(uint32_t)(exception_handler_end - exception_handler_begin));
	set_compare();
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
	init_enable_int();

	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		// do_scheduler();
	};
	return;
}
