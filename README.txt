�����ɫ:
1. init_pcb�з�����һ���ں��Լ���PCB��ʹcurrent_runningָ����
2. pcb��kernel_context.regs[0]��Ϊ�½����̵ı�־
3. �½����ָ̻�������ʱֻ��sp��ra�Ĵ�����ֵ
4. ÿ���ͷ�����������block_queue�ų�һ�����̲�ʹ������
5. ���󻥳���ֻ����һ������״̬

�޸ĵ��ļ�Ŀ¼�ṹ��
   |--arch/mips
   |   |--boot/bootblock.S       : ����ʵ��1��bootblock
   |   |--kernel/entry.S         : ���SAVE_CONTEXT��RESTORE_CONTEXT
   |--include/os
   |   |--lock.h                 : ���mutex_lock_t����
   |   |--sched.h                : ����pcb_t����
   |--init/main.c                : ���init_pcb
   |--kernel/
   |   |--sched/sched.c          : ���scheduler��do_block��do_unblock_one�������޸���do_unblock_one�ķ���ֵΪint
   |   |--locking/lock.c         : ��ɻ������ĳ�ʼ����������ͷ�
   |--tools/createimage.c        : ����ʵ��1��createimage
   |--README.txt                 : ���ĵ�