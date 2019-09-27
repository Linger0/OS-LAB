设计特色:
1. init_pcb中分配了一块内核自己的PCB并使current_running指向它
2. pcb中kernel_context.regs[0]作为新建进程的标志
3. 新建进程恢复上下文时只赋sp与ra寄存器的值
4. 每次释放锁会立即从block_queue放出一个进程并使其获得锁
5. 请求互斥锁只需检查一次锁的状态

修改的文件目录结构：
   |--arch/mips
   |   |--boot/bootblock.S       : 拷贝实验1的bootblock
   |   |--kernel/entry.S         : 完成SAVE_CONTEXT与RESTORE_CONTEXT
   |--include/os
   |   |--lock.h                 : 完成mutex_lock_t定义
   |   |--sched.h                : 完善pcb_t定义
   |--init/main.c                : 完成init_pcb
   |--kernel/
   |   |--sched/sched.c          : 完成scheduler、do_block与do_unblock_one，其中修改了do_unblock_one的返回值为int
   |   |--locking/lock.c         : 完成互斥锁的初始化、请求和释放
   |--tools/createimage.c        : 拷贝实验1的createimage
   |--README.txt                 : 本文档