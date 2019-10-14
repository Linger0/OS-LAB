
设计摘要:

main.c:	1.init_pcb中分配了一块内核自己的PCB并使current_running指向它
	2.初始化每个线程的优先级，初始优先级在开头定义的数值initial_priority中

entry.S:
	1.提供与CP0寄存器相关的函数(如get_cp0_count())用于C语言调用
	2.系统调用返回前CP0_EPC的值+4

lock.h\lock.c: 
	1.mutex_lock_t中添加了阻塞队列lock_queue, 每把锁拥有自己的阻塞队列
	2.lock_status同时作为锁当前允许的线程数，初始化时设为锁最大允许的线程数（Bonus）
	3.每次释放锁会立即从lock_queue放出一个进程并使其获得锁
	4.请求互斥锁只需检查一次锁的状态

sched.h\sched.c:
	1.pcb_t增加了唤起时间和优先级
	2.scheduler: 	a.在调度前检查sleep的队列block_queue
			b.重置当前进程的优先级
			c.如果是时钟中断，调度后重置时间片
	3.每次阻塞进程放出时立即增加将该进程的优先级

	注: 在scheduler中重置时间片，只有时钟中断时才会重置

lock_task2.c: 测试Bonus，详细说明见设计文档


修改的文件目录结构:

   |--arch/mips
   |   |--boot/bootblock.S            : 拷贝实验1的bootblock
   |   |--kernel
   |   |   |--entry.S                 : 完成异常处理(中断、系统调用)以及用于C语言调用的汇编函数 
   |   |   |--syscall.S               : 完成invoke_syscall()
   |--drivers
   |   |--screen.c                    : init_screen()中添加screen_clear()
   |--include/os 
   |   |--lock.h                      : 完成mutex_lock_t定义
   |   |--sched.h                     : 完善pcb_t定义
   |--init
   |   |--main.c                      : 完成init_pcb
   |--kernel/
   |   |--irq/irq.c                   : 完成中断判断interrupt_helper()和始终中断irq_timer()
   |   |--locking/lock.c              : 完成互斥锁的初始化、请求和释放
   |   |--sched
   |   |   |--sched.c                 : 完成scheduler、do_block与do_unblock_one, 其中修改了do_unblock_one的返回值为int
   |   |   |--time.c                  : get_tick()中更新time_elapsed
   |   |--syscall/syscall.c           : 去掉“syscall[fn](arg1, arg2, arg3)”前的注释
   |   |
   |--test/test_project2/test_lock2.c : 修改锁任务以测试Bonus
   |--tools/createimage.c             : 拷贝实验1的createimage
   |--README.txt                      : 本文档

