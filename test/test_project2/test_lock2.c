#include "test2.h"
#include "lock.h"
#include "stdio.h"
#include "syscall.h"

#define CYCLE 50
int is_init = FALSE;
static char blank[] = {"                                             "};

mutex_lock_t mutex_lock[NUM_LOCK];
lock_status_t lock_max_thread[NUM_LOCK] = {2,1};

void lock_task1(void)
{
        int print_location = 5;
        while (1)
        {
                int i;
                if (!is_init)
                {
					mutex_lock_init(mutex_lock);
                    is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock[0].\n");

                mutex_lock_acquire(&mutex_lock[0]);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock[1].\n");

                mutex_lock_acquire(&mutex_lock[1]);

                for (i = 0; i < CYCLE; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired 2 lock and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired 2 lock and exited.\n");

                mutex_lock_release(&mutex_lock[0]);
				
                mutex_lock_release(&mutex_lock[1]);
        }
}

void lock_task2(void)
{
        int print_location = 6;
        while (1)
        {
                int i;
                if (!is_init)
                {
					mutex_lock_init(mutex_lock);
                    is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock[0].\n");

                mutex_lock_acquire(&mutex_lock[0]);
				
                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock[1].\n");

                mutex_lock_acquire(&mutex_lock[1]);

                for (i = 0; i < CYCLE; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired 2 lock and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired 2 lock and exited.\n");

                mutex_lock_release(&mutex_lock[0]);
				
                mutex_lock_release(&mutex_lock[1]);
        }
}

void lock_task3(void)
{
        int print_location = 7;
        while (1)
        {
                int i;
                if (!is_init)
                {
					mutex_lock_init(mutex_lock);
                    is_init = TRUE;
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Applying for lock[0].\n");

                mutex_lock_acquire(&mutex_lock[0]);

                for (i = 0; i < CYCLE; i++)
                {
                        sys_move_cursor(1, print_location);
                        printf("> [TASK] Has acquired lock and running.(%d)\n", i);
                }

                sys_move_cursor(1, print_location);
                printf("%s", blank);

                sys_move_cursor(1, print_location);
                printf("> [TASK] Has acquired lock[0] and exited.\n");

                mutex_lock_release(&mutex_lock[0]);
        }
}