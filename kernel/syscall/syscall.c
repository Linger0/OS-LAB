#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "syscall.h"

void system_call_helper(int fn, int arg1, int arg2, int arg3)
{
    (current_running->user_context).regs[2] = syscall[fn](arg1, arg2, arg3);
}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

void sys_block(queue_t *queue)
{
    invoke_syscall(SYSCALL_BLOCK, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_one(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ONE, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_all(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ALL, (int)queue, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (int)buff, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void sys_clear(int line1, int line2)
{
    invoke_syscall(SYSCALL_CLEAR, line1, line2, IGNORE);
}

void mutex_lock_init(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_INIT, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_acquire(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_ACQUIRE, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_release(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_RELEASE, (int)lock, IGNORE, IGNORE);
}

void sys_spawn(task_info_t *task) 
{
    invoke_syscall(SYSCALL_SPAWN, (int)task, IGNORE, IGNORE);
}

void sys_kill(pid_t pid)
{
    invoke_syscall(SYSCALL_KILL, pid, IGNORE, IGNORE);
}

void sys_exit(void)
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}

void sys_waitpid(pid_t pid)
{
    invoke_syscall(SYSCALL_WAIT, pid, IGNORE, IGNORE);
}

void semaphore_init(semaphore_t *s, int val) 
{
    invoke_syscall(SYSCALL_SEMAPHORE_INIT, (int)s, val, IGNORE);
}

void semaphore_up(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_UP, (int)s, IGNORE, IGNORE);
}

void semaphore_down(semaphore_t *s)
{
    invoke_syscall(SYSCALL_SEMAPHORE_DOWN, (int)s, IGNORE, IGNORE);
}

void condition_init(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_INIT, (int)condition, IGNORE, IGNORE);
}

void condition_wait(mutex_lock_t *lock, condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_WAIT, (int)lock, (int)condition, IGNORE);
}

void condition_signal(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_SIGNAL, (int)condition, IGNORE, IGNORE);
}

void condition_broadcast(condition_t *condition)
{
    invoke_syscall(SYSCALL_CONDITION_BROADCAST, (int)condition, IGNORE, IGNORE);
}

void barrier_init(barrier_t *barrier, int goal)
{
    invoke_syscall(SYSCALL_BARRIER_INIT, (int)barrier, goal, IGNORE);
}

void barrier_wait(barrier_t *barrier)
{
    invoke_syscall(SYSCALL_BARRIER_WAIT, (int)barrier, IGNORE, IGNORE);
}

int sys_getpid(void)
{
    return invoke_syscall(SYSCALL_GETPID, IGNORE, IGNORE, IGNORE);
}

void sys_init_mac(void)
{
    invoke_syscall(SYSCALL_INIT_MAC, IGNORE, IGNORE, IGNORE);
}

void sys_net_send(uint32_t td, uint32_t td_phy)
{
    invoke_syscall(SYSCALL_NET_SEND, (int)td, (int)td_phy, IGNORE);
}

uint32_t sys_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{
    return (uint32_t)invoke_syscall(SYSCALL_NET_RECV, (int)rd, (int)rd_phy, (int)daddr);
}

void sys_wait_recv_package(void)
{
    invoke_syscall(SYSCALL_WAIT_RECV_PACKAGE, IGNORE, IGNORE, IGNORE);
}

void sys_mkfs(void)
{
    invoke_syscall(SYSCALL_MKFS, IGNORE, IGNORE, IGNORE);
}

void sys_fs_info(void)
{
    invoke_syscall(SYSCALL_FS_INFO, IGNORE, IGNORE, IGNORE);
}

void sys_mkdir(char *name)
{
    invoke_syscall(SYSCALL_MKDIR, (int)name, IGNORE, IGNORE);
}

void sys_rmdir(char *name)
{
    invoke_syscall(SYSCALL_RMDIR, (int)name, IGNORE, IGNORE);
}

void sys_read_dir(char *name)
{
    invoke_syscall(SYSCALL_READ_DIR, (int)name, IGNORE, IGNORE);
}

void sys_enter_fs(char *name)
{
    invoke_syscall(SYSCALL_ENTER_FS, (int)name, IGNORE, IGNORE);
}

void sys_mknod(char *fname)
{
    invoke_syscall(SYSCALL_MKNOD, (int)fname, IGNORE, IGNORE);
}

void sys_cat(char *fname)
{
    invoke_syscall(SYSCALL_CAT, (int)fname, IGNORE, IGNORE);
}

int sys_fopen(char *fname, uint32_t access)
{
    invoke_syscall(SYSCALL_FOPEN, (int)fname, (int)access, IGNORE);
}

int sys_fread(int fd, char *buff, int size)
{
    invoke_syscall(SYSCALL_FREAD, fd, (int)buff, size);
}

int sys_fwrite(int fd, char *data, int size)
{
    invoke_syscall(SYSCALL_FWRITE, fd, (int)data, size);
}

void sys_fclose(int fd)
{
    invoke_syscall(SYSCALL_FCLOSE, fd, IGNORE, IGNORE);
}

int sys_fseek(int fd, int pos)
{
    invoke_syscall(SYSCALL_FSEEK, fd, pos, IGNORE);
}