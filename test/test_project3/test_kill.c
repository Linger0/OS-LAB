#include "time.h"
#include "test3.h"
#include "lock.h"
#include "sched.h"
#include "stdio.h"
#include "syscall.h"

static char blank[] = {"                                                "};

mutex_lock_t lock1;
mutex_lock_t lock2;

static pid_t pid;

// pid = 2
void ready_to_exit_task()
{
    int i = 0, print_location = 0;
    pid = sys_getpid();

    mutex_lock_acquire(&lock1);
    mutex_lock_acquire(&lock2);

    // sys_spawn(&task1);
    // sys_spawn(&task2);
    if (pid == 2)
        for (i = 0; i < 500; i++)
        {
            sys_move_cursor(0, print_location);
            printf("> [TASK] I am task with pid %d, I have acquired two mutex lock. (%d)", pid, i++);
        }
    else 
        while (1)
        {
            sys_move_cursor(0, print_location);
            printf("> [TASK] I am task with pid %d, I have acquired two mutex lock. (%d)", pid, i++);
        }
    sys_exit(); // test exit
}

// pid = 3
void wait_lock_task()
{
    int i, print_location = 1;

    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to acquire a mute lock from task(pid=%d).", pid);

    mutex_lock_acquire(&lock1);

    sys_move_cursor(0, print_location);
    printf("> [TASK] I have acquired a mutex lock from task(pid=%d).", pid);

    sys_exit(); // test exit
}

// pid = 4
void wait_exit_task()
{
    int i, print_location = 2;

    sys_move_cursor(0, print_location);
    printf("> [TASK] I want to wait task (pid=%d) to exit.", pid);

    sys_waitpid(pid); //test waitpid

    sys_move_cursor(0, print_location);
    printf("> [TASK] Task (pid=%d) has exited.                ", pid);

    sys_exit(); // test exit
}