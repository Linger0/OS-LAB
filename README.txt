
���ժҪ:

main.c:	1.init_pcb�з�����һ���ں��Լ���PCB��ʹcurrent_runningָ����
	2.��ʼ��ÿ���̵߳����ȼ�����ʼ���ȼ��ڿ�ͷ�������ֵinitial_priority��

entry.S:
	1.�ṩ��CP0�Ĵ�����صĺ���(��get_cp0_count())����C���Ե���
	2.ϵͳ���÷���ǰCP0_EPC��ֵ+4

lock.h\lock.c: 
	1.mutex_lock_t���������������lock_queue, ÿ����ӵ���Լ�����������
	2.lock_statusͬʱ��Ϊ����ǰ������߳�������ʼ��ʱ��Ϊ�����������߳�����Bonus��
	3.ÿ���ͷ�����������lock_queue�ų�һ�����̲�ʹ������
	4.���󻥳���ֻ����һ������״̬

sched.h\sched.c:
	1.pcb_t�����˻���ʱ������ȼ�
	2.scheduler: 	a.�ڵ���ǰ���sleep�Ķ���block_queue
			b.���õ�ǰ���̵����ȼ�
			c.�����ʱ���жϣ����Ⱥ�����ʱ��Ƭ
	3.ÿ���������̷ų�ʱ�������ӽ��ý��̵����ȼ�

	ע: ��scheduler������ʱ��Ƭ��ֻ��ʱ���ж�ʱ�Ż�����

lock_task2.c: ����Bonus����ϸ˵��������ĵ�


�޸ĵ��ļ�Ŀ¼�ṹ:

   |--arch/mips
   |   |--boot/bootblock.S            : ����ʵ��1��bootblock
   |   |--kernel
   |   |   |--entry.S                 : ����쳣����(�жϡ�ϵͳ����)�Լ�����C���Ե��õĻ�ຯ�� 
   |   |   |--syscall.S               : ���invoke_syscall()
   |--drivers
   |   |--screen.c                    : init_screen()�����screen_clear()
   |--include/os 
   |   |--lock.h                      : ���mutex_lock_t����
   |   |--sched.h                     : ����pcb_t����
   |--init
   |   |--main.c                      : ���init_pcb
   |--kernel/
   |   |--irq/irq.c                   : ����ж��ж�interrupt_helper()��ʼ���ж�irq_timer()
   |   |--locking/lock.c              : ��ɻ������ĳ�ʼ����������ͷ�
   |   |--sched
   |   |   |--sched.c                 : ���scheduler��do_block��do_unblock_one, �����޸���do_unblock_one�ķ���ֵΪint
   |   |   |--time.c                  : get_tick()�и���time_elapsed
   |   |--syscall/syscall.c           : ȥ����syscall[fn](arg1, arg2, arg3)��ǰ��ע��
   |   |
   |--test/test_project2/test_lock2.c : �޸��������Բ���Bonus
   |--tools/createimage.c             : ����ʵ��1��createimage
   |--README.txt                      : ���ĵ�

