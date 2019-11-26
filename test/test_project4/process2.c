#include "sched.h"
#include "stdio.h"
#include "syscall.h"
#include "time.h"
#include "screen.h"
#include "test4.h"

#define RW_TIMES 3

int rand()
{	
	int current_time = get_timer();
	return current_time % 100000;
}

/*static void disable_interrupt()
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
*/

unsigned long a[6];

void rw_task1()
{
	int mem1, mem2 = 0;
	int memory[RW_TIMES];
	int i = 0;
	sys_move_cursor(0,0);
	for(i = 0; i < RW_TIMES; i++)
	{
		mem1 = a[i];
		memory[i] = mem2 = rand();
		*(int *)mem1 = mem2;
		printf("> Write: 0x%x, %d\n", mem1, mem2);
	}
	for(i = 0; i < RW_TIMES; i++)
	{
		mem1 = a[RW_TIMES+i];
		memory[i+RW_TIMES] = *(int *)mem1;
		if(memory[i+RW_TIMES] == memory[i])
			printf("> Read succeed: %d\n", memory[i+RW_TIMES]);
		else
			printf("> Read error: %d\n", memory[i+RW_TIMES]);
	}
	sys_exit();
	//Input address from argv.
}
