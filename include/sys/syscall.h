/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                       System call related processing
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
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

#ifndef INCLUDE_SYSCALL_H_
#define INCLUDE_SYSCALL_H_

#include "type.h"
#include "sync.h"
#include "queue.h"
#include "sched.h"

#define IGNORE 0
#define NUM_SYSCALLS 64

/* define */
#define SYSCALL_SLEEP 2

#define SYSCALL_BLOCK 10
#define SYSCALL_UNBLOCK_ONE 11
#define SYSCALL_UNBLOCK_ALL 12

#define SYSCALL_WRITE 20
#define SYSCALL_READ 21
#define SYSCALL_CURSOR 22
#define SYSCALL_REFLUSH 23
#define SYSCALL_CLEAR 24

#define SYSCALL_MUTEX_LOCK_INIT 30
#define SYSCALL_MUTEX_LOCK_ACQUIRE 31
#define SYSCALL_MUTEX_LOCK_RELEASE 32

#define SYSCALL_SPAWN 25
#define SYSCALL_KILL 26
#define SYSCALL_EXIT 27
#define SYSCALL_WAIT 28

#define SYSCALL_SEMAPHORE_INIT 35
#define SYSCALL_SEMAPHORE_UP 36
#define SYSCALL_SEMAPHORE_DOWN 37

#define SYSCALL_CONDITION_INIT 40
#define SYSCALL_CONDITION_WAIT 41
#define SYSCALL_CONDITION_SIGNAL 42
#define SYSCALL_CONDITION_BROADCAST 43

#define SYSCALL_BARRIER_INIT 45
#define SYSCALL_BARRIER_WAIT 46

/* syscall function pointer */
int (*syscall[NUM_SYSCALLS])();

void system_call_helper(int, int, int, int);
extern int invoke_syscall(int, int, int, int);

void sys_sleep(uint32_t);

void sys_block(queue_t *);
void sys_unblock_one(queue_t *);
void sys_unblock_all(queue_t *);

void sys_write(char *);
void sys_move_cursor(int, int);
void sys_reflush(void);
void sys_clear(int, int);

void mutex_lock_init(mutex_lock_t *);
void mutex_lock_acquire(mutex_lock_t *);
void mutex_lock_release(mutex_lock_t *);

void sys_spawn(task_info_t *);
void sys_kill(pid_t);
void sys_exit(void);
void sys_waitpid(pid_t);

void semaphore_init(semaphore_t *, int);
void semaphore_up(semaphore_t *);
void semaphore_down(semaphore_t *);

void condition_init(condition_t *);
void condition_wait(mutex_lock_t *, condition_t *);
void condition_signal(condition_t *);
void condition_broadcast(condition_t *);

void barrier_init(barrier_t *, int);
void barrier_wait(barrier_t *);

int sys_getpid(void);

#endif