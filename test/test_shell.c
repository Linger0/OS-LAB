/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
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

#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "irq.h"

static void disable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status &= 0xfffffffe;
    set_cp0_status(cp0_status);
}

static void enable_interrupt()
{
    uint32_t cp0_status = get_cp0_status();
    cp0_status |= 0x01;
    set_cp0_status(cp0_status);
}

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

int atoi(char *str) {
    int i = 0;
    while (*str >= '0' && *str <= '9') {
        i = 10 * i + *str - '0';
        str++;
    }
    return i;
}

struct task_info task1 = {"task1", (uint32_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {"task3", (uint32_t)&wait_exit_task, USER_PROCESS};

struct task_info task4 = {"task4", (uint32_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task5 = {"task5", (uint32_t)&semaphore_add_task2, USER_PROCESS};
struct task_info task6 = {"task6", (uint32_t)&semaphore_add_task3, USER_PROCESS};

struct task_info task7 = {"task7", (uint32_t)&producer_task, USER_PROCESS};
struct task_info task8 = {"task8", (uint32_t)&consumer_task1, USER_PROCESS};
struct task_info task9 = {"task9", (uint32_t)&consumer_task2, USER_PROCESS};

struct task_info task10 = {"task10", (uint32_t)&barrier_task1, USER_PROCESS};
struct task_info task11 = {"task11", (uint32_t)&barrier_task2, USER_PROCESS};
struct task_info task12 = {"task12", (uint32_t)&barrier_task3, USER_PROCESS};

struct task_info task13 = {"SunQuan", (uint32_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {"LiuBei", (uint32_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {"CaoCao", (uint32_t)&CaoCao, USER_PROCESS};

static struct task_info *test_tasks[16] = {&task1, &task2, &task3,
                                           &task4, &task5, &task6,
                                           &task7, &task8, &task9,
                                           &task10, &task11, &task12,
                                           &task13, &task14, &task15};
static int num_test_tasks = 15;

static char split_line[] = "------------------- COMMAND -------------------";
static char root[] = "> Linger@myOS: ";

static char *status_str[4] = {
    "TASK_BLOCKED",
    "TASK_RUNNING",
    "TASK_READY",
    "TASK_EXITED"
};

void test_shell()
{
    int index = 0;
    char Cmd[CMD_LEN];

    sys_move_cursor(0, SPLIT_LOC);
    printf(split_line);

    sys_move_cursor(0, SPLIT_LOC + 1);
    printf(root);

    while (1)
    {
        // read command from UART port
        disable_interrupt();
        char ch = read_uart_ch();
        enable_interrupt();

        // record to Cmd[]
        if (ch) {
            if (ch == '\b') {
                if (index > 0) {
                    --index;
                    printf("%c", ch);
                }
            }
            else {
                Cmd[index++] = ch;
                printf("%c", ch);
            }
        }

        if (ch == '\r') { // solve command
            if (--index > 0) { // Cmd[] is not empty
                Cmd[index] = '\0';
                if (!strcmp(Cmd, "ps")) { // ps
                    int found, i, status;
                    found = 0;
                    for (i = 0; i < NUM_MAX_TASK; i++) {
                        if (pcb[i].status != TASK_EXITED) {
                            status = pcb[i].status;
                            printf("[%d] PID: %d  STATUS: %s\n", 
                                    found, pcb[i].pid, status_str[status]);
                            found++;
                        }
                    }
                }
                else if (!strcmp(Cmd, "clear")) { // clear
                    sys_clear(0, SPLIT_LOC - 1);
                    sys_clear(SPLIT_LOC + 1, SCREEN_HEIGHT - 1);
                    sys_move_cursor(0, SPLIT_LOC + 1);
                }
                else if (strcmp(Cmd, "exec") == (' ' - '\0')) { // exec
                    int n = atoi(Cmd + 5);
                    printf("[EXEC] Process[%d].\n", n);
                    sys_spawn(test_tasks[n]);
                }
                else if (strcmp(Cmd, "kill") == (' ' - '\0')) { // kill
                    int pid = atoi(Cmd + 5);
                    if (pid == 1) printf("[ERROR] Can't kill shell.\n");
                    else {
                        printf("[KILL] Process pid=%d.\n", pid);
                        sys_kill(pid);
                    }
                }
                else { // other
                    printf("[ERROR] Unknown command.\n");
                }
            }
            printf(root);
            index = 0;
        }
    }
}