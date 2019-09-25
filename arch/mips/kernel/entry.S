#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

.macro STI
.endm

.macro CLI
.endm

LEAF(asm_start)
    mtc0    zero, CP0_STATUS
    mtc0    zero, CP0_WATCHLO
    mtc0    zero, CP0_WATCHHI

    mfc0    t0, CP0_CONFIG
    and     t0, ~0x7
    ori     t0, 0x2
    mtc0    t0, CP0_CONFIG
    
    jr  ra
END(asm_start)

.macro SAVE_CONTEXT offset
    	// TODO save context
	addiu	sp, sp, -8
	sw	ra, 4(sp)		# ra ֵ入栈
	lw	ra, current_running 	# ra 做为PCB基址
	addiu	ra, ra, \offset
	sw	$0, (ra)
	sw	$1, 4(ra)
	sw	$2, 8(ra)
	sw	$3, 12(ra)
	sw	$4, 16(ra)
	sw	$5, 20(ra)
	sw	$6, 24(ra)
	sw	$7, 28(ra)
	sw	$8, 32(ra)
	sw	$9, 36(ra)
	sw	$10, 40(ra)
	sw	$11, 44(ra)
	sw	$12, 48(ra)
	sw	$13, 52(ra)
	sw	$14, 56(ra)
	sw	$15, 60(ra)
	sw	$16, 64(ra)
	sw	$17, 68(ra)
	sw	$18, 72(ra)
	sw	$19, 76(ra)
	sw	$20, 80(ra)
	sw	$21, 84(ra)
	sw	$22, 88(ra)
	sw	$23, 92(ra)
	sw	$24, 96(ra)
	sw	$25, 100(ra)
	sw	$26, 104(ra)
	sw	$27, 108(ra)
	sw	$28, 112(ra)
	sw	$30, 120(ra)
	move 	t2, ra
	lw	ra, 4(sp)	# ra 出栈
	addiu	sp, sp, 8	# 回调 sp
	sw	$29, 116(t2)
	sw	$31, 124(t2)
.endm

.macro RESTORE_CONTEXT offset
    	// TODO restore context
	lw	ra, current_running # ra 作为PCB基址
	lw	t2, (ra)
	addi	ra, ra, \offset
	bnez	t2, new		# regs[0]==1 跳转（新进程）

	lw	$1, 4(ra)
	lw	$2, 8(ra)
	lw	$3, 12(ra)
	lw	$4, 16(ra)
	lw	$5, 20(ra)
	lw	$6, 24(ra)
	lw	$7, 28(ra)
	lw	$8, 32(ra)
	lw	$9, 36(ra)
	lw	$10, 40(ra)
	lw	$11, 44(ra)
	lw	$12, 48(ra)
	lw	$13, 52(ra)
	lw	$14, 56(ra)
	lw	$15, 60(ra)
	lw	$16, 64(ra)
	lw	$17, 68(ra)
	lw	$18, 72(ra)
	lw	$19, 76(ra)
	lw	$20, 80(ra)
	lw	$21, 84(ra)
	lw	$22, 88(ra)
	lw	$23, 92(ra)
	lw	$24, 96(ra)
	lw	$25, 100(ra)
	lw	$26, 104(ra)
	lw	$27, 108(ra)
	lw	$28, 112(ra)
	lw	$30, 120(ra)
new:	lw	$29, 116(ra)	# 新进程只修改 sp, ra
	lw	$31, 124(ra)
.endm

NESTED(do_scheduler, 0, ra)
	SAVE_CONTEXT(KERNEL)
	jal     scheduler
	RESTORE_CONTEXT(KERNEL)
	jr      ra	
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    // TODO close interrupt
    // jmp exception_handler[i] which decided by CP0_CAUSE
    // Leve2 exception Handler.
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt handler
    // Leve3 exception Handler.
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
END(handle_other)


