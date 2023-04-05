# Tiny OS

### Project 6 - File System

文件块索引 (支持 256MB 大文件):

1. 9 个直接索引指针 (9*4KB=36KB)
2. 2 个间接索引指针 (2*1024*128KB=256MB)

    1. 指针索引间址块 (1024 个数据块指针项)
    2. 间接索引数据块为大块 (chunk=32*block=32*4KB=128KB)

主要文件目录结构:

```
   |--include/os/
   |   |--fs.h                        : 文件系统头文件（包含磁盘布局和数据结构）
   |--kernel/fs/
   |   |--filesystem.c                : 文件系统、目录和文件操作
   |--test/
   |   |--test_shell.c                : shell
   |   |--test_fs/
   |   |   |--test_fs.c               : 增加两个 task 测试大文件
   |--README.txt                      : 本文档
```

### Project 5 - Device Driver

文件目录结构与修改说明:

```
   |--drivers/
   |   |--mac.h                       : 包含描述符、buffer 基址宏定义
   |   |--mac.c                       : mac_irq_handle()、mac_recv_handle()、do_net_recv()、do_net_send()
   |--test/test_net/
   |   |--test_mac3.c                 : 初始化描述符函数
   |--README.txt                      : 本文档
```

### Project 4 - Virtual Memory

设计摘要:

1. 采用二级页表，虚拟地址结构如下:
    ```
     ----------------------
     |31..22|21..12| 11..0|
     ----------------------
     |  PDN |  VPN |OFFSET|
     ----------------------
    ```
2. 页表分配在地址空间内核段
    - 页目录: 0xa0101000-0xa0111000
    - 页表: 0xa0111000-0xa0200000

3. 页目录在系统启动时分配并初始化，与 PCB 绑定


文件目录结构与修改说明:
```
   |--arch/mips/kernel/
   |   |--entry.S                     : TLB Refill、TLB Invalid 例外处理
   |--include/os/
   |   |--sched.h                     : PCB 添加 pde_base
   |   |--mm.h                        : 页目录项与页表项数据类型、内存管理相关函数与宏定义
   |--init/
   |   |--main.c                      : 初始化内存管理
   |--kernel/
   |   |--/mm/mm.c                    : 内存管理
   |   |--/sched/sched.c              : （1）scheduler() 添加修改 CP0_EntryHi 的 ASID 代码
   |   |                                （2）spawn() 添加页目录初始化代码
   |--test
   |   |--test_shell.c                : Shell
   |--README.txt                      : 本文档
```

### Project 3 - Interactive OS Process Mgt

文件目录结构与修改说明:
```
   |--include/os/
   |   |--sched.h                     : PCB 添加 wait_pid、lock_queue、belong
   |   |--barrier.h                   : barrier_t
   |   |--cond.h                      : condition_t
   |   |--sem.h                       : semaphore_t
   |--kernel/
   |   |--locking/
   |   |   |--barrier.c               : barrier
   |   |   |--cond.c                  : condition
   |   |   |--sem.c                   : semaphore
   |   |--sched/
   |   |   |--sched.c                 : do_exit()、do_kill()、do_spawn()、do_waitpid()
   |   |   |--queue.c                 : 1. push、dequeue、remove 添加对 belong 的赋值
   |   |   |                            2. 添加 lqueue_push、lqueue_dequeue 函数 (对 PCB 中锁队列 lock_queue 的操作)
   |   |--syscall/
   |   |   |--syscall.c               : 封装系统调用
   |--libs/
   |   |--mailbox.h                   : mailbox_t
   |   |--mailbox.c                   : mailbox
   |--test/
   |   |--test_shell.c                : shell
   |--README.txt                      : 本文档
```

### Project 1 & Project 2 - Bootloader & Simple Kernel

设计摘要:

main.c:
1. init_pcb 中分配了一块内核自己的 PCB 并使 current_running 指向它
2. 初始化每个线程的优先级，初始优先级在开头定义的数值 initial_priority 中

entry.S:
1. 提供与 CP0 寄存器相关的函数 (如 get_cp0_count) 用于 C 语言调用
2. 系统调用返回前 CP0_EPC 的值 + 4

lock.h\lock.c:
1. mutex_lock_t 中添加了阻塞队列 block_queue, 每把锁拥有自己的阻塞队列
2. lock_status 同时作为锁当前允许的线程数，初始化时设为锁最大允许的线程数（Bonus）
3. 每次释放锁会立即从 block_queue 放出一个进程并使其获得锁
4. 请求互斥锁只需检查一次锁的状态

sched.h\sched.c:
1. pcb_t 增加了唤起时间和优先级
2. scheduler:
	1. 在调度前检查 sleep 的队列 sleep_queue
	2. 重置当前进程的优先级
	3. 如果是时钟中断，调度后重置时间片
3. 每次阻塞进程放出时立即增加将该进程的优先级

注：在 scheduler 中重置时间片，只有时钟中断时才会重置

lock_task2.c:
1. 测试 Bonus，详细说明见设计文档


修改的文件目录结构:
```
   |--arch/mips/
   |   |--boot/bootblock.S            : 拷贝实验 1 的 bootblock
   |   |--kernel/
   |   |   |--entry.S                 : 完成异常处理 (中断、系统调用) 以及用于 C 语言调用的汇编函数
   |   |   |--syscall.S               : 完成 invoke_syscall()
   |--drivers/
   |   |--screen.c                    : init_screen() 中添加 screen_clear()
   |--include/os/
   |   |--lock.h                      : 完成 mutex_lock_t 定义
   |   |--sched.h                     : 完善 pcb_t 定义
   |--init/
   |   |--main.c                      : 完成 init_pcb
   |--kernel/
   |   |--irq/irq.c                   : 完成中断判断 interrupt_helper() 和始终中断 irq_timer()
   |   |--locking/lock.c              : 完成互斥锁的初始化、请求和释放
   |   |--sched/
   |   |   |--sched.c                 : 完成 scheduler、do_block 与 do_unblock_one, 其中修改了 do_unblock_one 的返回值为 int
   |   |   |--time.c                  : get_tick() 中更新 time_elapsed
   |   |--syscall/syscall.c           : 去掉 “syscall[fn](arg1, arg2, arg3)” 前的注释
   |   |
   |--test/test_project2/test_lock2.c : 修改锁任务以测试 Bonus
   |--tools/createimage.c             : 拷贝实验 1 的 createimage
   |--README.txt                      : 本文档
```
