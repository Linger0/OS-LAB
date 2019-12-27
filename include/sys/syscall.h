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

#define SYSCALL_WRITE 15
#define SYSCALL_READ 16
#define SYSCALL_CURSOR 17
#define SYSCALL_REFLUSH 18
#define SYSCALL_CLEAR 19

#define SYSCALL_SPAWN 20
#define SYSCALL_KILL 21
#define SYSCALL_EXIT 22
#define SYSCALL_WAIT 23

#define SYSCALL_GETPID 24

#define SYSCALL_MUTEX_LOCK_INIT 25
#define SYSCALL_MUTEX_LOCK_ACQUIRE 26
#define SYSCALL_MUTEX_LOCK_RELEASE 27

#define SYSCALL_SEMAPHORE_INIT 28
#define SYSCALL_SEMAPHORE_UP 29
#define SYSCALL_SEMAPHORE_DOWN 30

#define SYSCALL_CONDITION_INIT 31
#define SYSCALL_CONDITION_WAIT 32
#define SYSCALL_CONDITION_SIGNAL 33
#define SYSCALL_CONDITION_BROADCAST 34

#define SYSCALL_BARRIER_INIT 35
#define SYSCALL_BARRIER_WAIT 36

#define SYSCALL_INIT_MAC 40
#define SYSCALL_NET_SEND 41
#define SYSCALL_NET_RECV 42
#define SYSCALL_WAIT_RECV_PACKAGE 43

#define SYSCALL_MKFS 45
#define SYSCALL_MKDIR 46 
#define SYSCALL_RMDIR 47
#define SYSCALL_READ_DIR 48
#define SYSCALL_FS_INFO 49
#define SYSCALL_ENTER_FS 50

#define SYSCALL_MKNOD 51
#define SYSCALL_CAT 52
#define SYSCALL_FOPEN 53
#define SYSCALL_FREAD 54
#define SYSCALL_FWRITE 55
#define SYSCALL_FCLOSE 56

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

int sys_getpid(void);

void semaphore_init(semaphore_t *, int);
void semaphore_up(semaphore_t *);
void semaphore_down(semaphore_t *);

void condition_init(condition_t *);
void condition_wait(mutex_lock_t *, condition_t *);
void condition_signal(condition_t *);
void condition_broadcast(condition_t *);

void barrier_init(barrier_t *, int);
void barrier_wait(barrier_t *);

void sys_init_mac(void);
void sys_net_send(uint32_t, uint32_t);
uint32_t sys_net_recv(uint32_t, uint32_t, uint32_t);
void sys_wait_recv_package(void);

void sys_mkfs();
void sys_mkdir(char *);
void sys_rmdir(char *);
void sys_read_dir(char *);
void sys_fs_info();
void sys_enter_fs(char *);

void sys_mknod();
void sys_cat();
int sys_fopen(char *name, uint32_t rw);
void sys_fread(int fd, char *buff, int cnt);
void sys_fwrite(int fd, char *data, int cnt);
void sys_close(int fd);

#endif