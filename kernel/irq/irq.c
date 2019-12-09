#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"
#include "mac.h"

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
    if ((status & cause & IPL) == 0x8000) { // 时钟中断处理
        irq_timer();
    }
    else if ((status & cause & 0x800) == 0x800) {   // 设备中断处理
        if ((read_register(Int1_SR, 0) & 0x8) == 0x8) {
            mac_irq_handle();
        }
        else {
            other_exception_handler();
        }
    }
    else {
        other_exception_handler();
    }
    return;
}