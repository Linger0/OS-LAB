#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"

int rst_timer = 0;

static void irq_timer()
{
    screen_reflush();
    rst_timer = 1;
    do_scheduler(); // 由 scheduler 重置 cp0_count & cp0_compare
    return;
}

void other_exception_handler()
{
    return;
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    // Leve3 exception Handler.
    if ((status & cause & IP) == 0x8000)
        irq_timer();
    else 
        other_exception_handler();
    return;
}