#include "asm.h"

#define TIME_INTERVAL 150000

.equ    KERNEL, 0
.equ    USER,   156

.macro DISABLE_INT
    mfc0    k0, CP0_STATUS
    li      k1, ~STATUS_IE
    and     k0, k0, k1
    mtc0    k0, CP0_STATUS
.endm

.macro ENABLE_INT
    mfc0    k0, CP0_STATUS
    ori     k0, STATUS_IE
    mtc0    k0, CP0_STATUS
.endm

.macro QUIT_INT
    RESTORE_CONTEXT(USER)
    eret
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
.set noat
    // TODO save context
    lw		k1, current_running 	# k1 做为PCB基址
    addiu	k1, k1, \offset
    sw		zero, OFFSET_REG0(k1)
    sw		AT, OFFSET_REG1(k1)
    sw	    v0, OFFSET_REG2(k1)
    sw	    v1, OFFSET_REG3(k1)
    sw	    a0, OFFSET_REG4(k1)
    sw	    a1, OFFSET_REG5(k1)
    sw	    a2, OFFSET_REG6(k1)
    sw	    a3, OFFSET_REG7(k1)
    sw	    t0, OFFSET_REG8(k1)
    sw	    t1, OFFSET_REG9(k1)
    sw	    t2, OFFSET_REG10(k1)
    sw	    t3, OFFSET_REG11(k1)
    sw	    t4, OFFSET_REG12(k1)
    sw	    t5, OFFSET_REG13(k1)
    sw	    t6, OFFSET_REG14(k1)
    sw	    t7, OFFSET_REG15(k1)
    sw	    s0, OFFSET_REG16(k1)
    sw	    s1, OFFSET_REG17(k1)
    sw	    s2, OFFSET_REG18(k1)
    sw	    s3, OFFSET_REG19(k1)
    sw	    s4, OFFSET_REG20(k1)
    sw	    s5, OFFSET_REG21(k1)
    sw	    s6, OFFSET_REG22(k1)
    sw	    s7, OFFSET_REG23(k1)
    sw	    t8, OFFSET_REG24(k1)
    sw	    t9, OFFSET_REG25(k1)
    sw	    gp, OFFSET_REG28(k1)
    sw	    sp, OFFSET_REG29(k1)
    sw  	fp, OFFSET_REG30(k1)
    sw	    ra, OFFSET_REG31(k1)
    
    mfc0    k0, CP0_STATUS
    sw      k0, OFFSET_STATUS(k1)
    mfhi    k0
    sw      k0, OFFSET_HI(k1)
    mflo    k0
    sw		k0, OFFSET_LO(k1)
    mfc0    k0, CP0_BADVADDR
    sw      k0, OFFSET_BADVADDR(k1)
    mfc0    k0, CP0_CAUSE
    sw      k0, OFFSET_CAUSE(k1)
    mfc0    k0, CP0_EPC
    sw      k0, OFFSET_EPC(k1)
.endm

.macro RESTORE_CONTEXT offset
.set noat
    // TODO restore context
    lw	    k1, current_running 	# k1 作为PCB基址
    addi	k1, k1, \offset
    lw      t1, (k1)
    bnez    t1, 1f                  # regs[0]==1 jmp
    lw	    AT, OFFSET_REG1(k1)
    lw	    v0, OFFSET_REG2(k1)
    lw	    v1, OFFSET_REG3(k1)
    lw	    a0, OFFSET_REG4(k1)
    lw	    a1, OFFSET_REG5(k1)
    lw	    a2, OFFSET_REG6(k1)
    lw	    a3, OFFSET_REG7(k1)
    lw	    t0, OFFSET_REG8(k1)
    lw	    t1, OFFSET_REG9(k1)
    lw	    t2, OFFSET_REG10(k1)
    lw	    t3, OFFSET_REG11(k1)
    lw	    t4, OFFSET_REG12(k1)
    lw	    t5, OFFSET_REG13(k1)
    lw	    t6, OFFSET_REG14(k1)
    lw	    t7, OFFSET_REG15(k1)
    lw	    s0, OFFSET_REG16(k1)
    lw	    s1, OFFSET_REG17(k1)
    lw	    s2, OFFSET_REG18(k1)
    lw	    s3, OFFSET_REG19(k1)
    lw	    s4, OFFSET_REG20(k1)
    lw	    s5, OFFSET_REG21(k1)
    lw	    s6, OFFSET_REG22(k1)
    lw	    s7, OFFSET_REG23(k1)
    lw	    t8, OFFSET_REG24(k1)
    lw	    t9, OFFSET_REG25(k1)
    lw	    gp, OFFSET_REG28(k1)
    lw	    fp, OFFSET_REG30(k1)
    lw      k0, OFFSET_STATUS(k1)
    mtc0    k0, CP0_STATUS
1:
    lw		k0, OFFSET_HI(k1)
    mthi	k0
    lw		k0, OFFSET_LO(k1)
    mtlo	k0
    lw		k0, OFFSET_BADVADDR(k1)
    mtc0	k0, CP0_BADVADDR
    lw		k0, OFFSET_CAUSE(k1)
    mtc0	k0, CP0_CAUSE
    lw	    ra, OFFSET_REG31(k1)
    lw	    sp, OFFSET_REG29(k1)
    lw		k0, OFFSET_EPC(k1)
    mtc0	k0, CP0_EPC
.endm

LEAF(init_status)
    mfc0	k0, CP0_STATUS
    or	    k0, k0, a0
    li      k1, ~STATUS_IE
    and     k0, k0, k1    
    mtc0	k0, CP0_STATUS
    jr      ra
END(init_status)

LEAF(init_enable_int)
    ENABLE_INT
    jr      ra
END(init_enable_int)

LEAF(new_proc_run)
    QUIT_INT
END(new_proc_run)

LEAF(get_count)
    mfc0    v0, CP0_COUNT
    jr      ra
END(get_count)

LEAF(set_compare)
    mtc0    zero, CP0_COUNT
    li      k0, TIME_INTERVAL
    mtc0    k0, CP0_COMPARE
    jr      ra
END(set_compare)

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    mtc0    zero, CP0_COUNT
    li      k0, TIME_INTERVAL
    mtc0    k0, CP0_COMPARE
    jr      ra	
END(do_scheduler)

.global exception_handler_begin
.global exception_handler_end

NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    // TODO close interrupt
    // Leve2 exception Handler.
    // jmp exception_handler[i] which decided by CP0_CAUSE
    SAVE_CONTEXT(USER)
    mfc0    k1, CP0_CAUSE
    and     k1, k1, CAUSE_EXCCODE
    srl     k1, k1, 2
    la      k0, exception_handler
    add     k0, k0, k1
    lw      ra, (k0)
    jr      ra
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt Handler
    // Leve3 exception Handler.
    mfc0    a0, CP0_STATUS
    mfc0    a1, CP0_CAUSE
    jal     interrupt_helper
    QUIT_INT
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
    QUIT_INT
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
    QUIT_INT
END(handle_other)

LEAF(disable_int)
    DISABLE_INT
    jr  ra
END(disable_int)

LEAF(enable_int)
    ENABLE_INT
    jr  ra
END(enable_int)
