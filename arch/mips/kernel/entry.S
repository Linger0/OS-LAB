#include "asm.h"

.equ    KERNEL, 0
.equ    USER,   156

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
    // save context
    lw      k1, current_running 	# k1 做为PCB基址
    addiu   k1, k1, \offset
    sw      AT, OFFSET_REG1(k1)
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
    sw      fp, OFFSET_REG30(k1)
    sw	    ra, OFFSET_REG31(k1)
    
    mfc0    k0, CP0_STATUS
    sw      k0, OFFSET_STATUS(k1)
    mfhi    k0
    sw      k0, OFFSET_HI(k1)
    mflo    k0
    sw      k0, OFFSET_LO(k1)
    mfc0    k0, CP0_BADVADDR
    sw      k0, OFFSET_BADVADDR(k1)
    mfc0    k0, CP0_CAUSE
    sw      k0, OFFSET_CAUSE(k1)
    mfc0    k0, CP0_EPC
    sw      k0, OFFSET_EPC(k1)
.set at
.endm

.macro RESTORE_CONTEXT offset
.set noat
    // restore context
    lw	    k1, current_running 	# k1 作为PCB基址
    addi    k1, k1, \offset
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
    lw	    sp, OFFSET_REG29(k1)
    lw	    fp, OFFSET_REG30(k1)
    lw	    ra, OFFSET_REG31(k1)

    lw      k0, OFFSET_STATUS(k1)
    mtc0    k0, CP0_STATUS
    lw      k0, OFFSET_HI(k1)
    mthi    k0
    lw      k0, OFFSET_LO(k1)
    mtlo    k0
    lw      k0, OFFSET_BADVADDR(k1)
    mtc0    k0, CP0_BADVADDR
    lw      k0, OFFSET_CAUSE(k1)
    mtc0    k0, CP0_CAUSE
    lw      k0, OFFSET_EPC(k1)
    mtc0    k0, CP0_EPC
.set at
.endm

NESTED(do_scheduler, 0, ra)
    SAVE_CONTEXT(KERNEL)
    jal     scheduler
    RESTORE_CONTEXT(KERNEL)
    jr      ra	
END(do_scheduler)

# .global TLBexception_handler_begin
# .global TLBexception_handler_end
.global exception_handler_begin
.global exception_handler_end
/*
NESTED(TLBexception_handler_entry, 0, sp)
.set noat
TLBexception_handler_begin:
	// TLB Refill
    lw      k0, current_running
    lw      k0, 320(k0)         // PDEBase
    mfc0    k1, CP0_ENTRYHI
    srl     k1, k1, 22          
    sll     k1, k1, 2           // PDN || 00
    or      k1, k1, k0          // PDE = PDEBase || PDN || 00
    lw      k1, (k1)            // PTEBASE
    mfc0    k0, CP0_ENTRYHI
    srl     k0, k0, 10
    andi    k0, k0, 0xff8       // PTN || 00
    or      k1, k1, k0          // PTE = PTEBASE || PTN || 00
    lw      k0, (k1)
    lw      k1, 4(k1)
    mtc0    k0, CP0_ENTRYLO0
    mtc0    k1, CP0_ENTRYLO1
    tlbwr
    eret
TLBexception_handler_end:
.set at
END(TLBexception_handler_entry)
*/
NESTED(exception_handler_entry, 0, sp)   
exception_handler_begin:
    // Leve2 exception Handler.
    // jmp exception_handler[i] which decided by CP0_CAUSE
    SAVE_CONTEXT(USER)
    lw      sp, OFFSET_SIZE(k1) // 内核栈栈顶
    mfc0    k1, CP0_CAUSE
    and     k1, k1, CAUSE_EXCCODE
    la      k0, exception_handler
    add     k0, k0, k1
    lw      k1, (k0)
    jr      k1
exception_handler_end:
END(exception_handler_entry)

NESTED(handle_int, 0, sp)
    // interrupt Handler
    // Leve3 exception Handler.
    mfc0    a0, CP0_STATUS
    mfc0    a1, CP0_CAUSE
    addiu   sp, sp, -8
    jal     interrupt_helper
    addiu   sp, sp, 8
    RESTORE_CONTEXT(USER)
    eret
END(handle_int)

NESTED(handle_syscall, 0, sp)
    // system call handler
    addiu   sp, sp, -16
    jal	    system_call_helper
    addiu   sp, sp, 16
    RESTORE_CONTEXT(USER)
    mfc0    k0, CP0_EPC
    add     k0, 4
    mtc0    k0, CP0_EPC
    eret
END(handle_syscall)

NESTED(handle_other, 0, sp)
    // other exception handler
    RESTORE_CONTEXT(USER)
    eret
END(handle_other)
/*
NESTED(handle_TLBinvalid, 0, sp)
    mfc0    a0, CP0_BADVADDR
    addiu   sp, sp, -4
    jal     do_TLBinvalid
    addiu   sp, sp, 4
    tlbp
    tlbwi
    RESTORE_CONTEXT(USER)
    eret
END(handle_TLBinvalid)
*/
LEAF(new_proc_run)
    // New process start here.
    RESTORE_CONTEXT(USER)
    eret
END(new_proc_run)

LEAF(get_cp0_status)
    // Set CP0_STATUS.
    mfc0    v0, CP0_STATUS
    jr      ra
END(get_cp0_status)

LEAF(set_cp0_status)
    // Set CP0_STATUS.
    mtc0    a0, CP0_STATUS
    jr      ra
END(set_cp0_status)

LEAF(get_cp0_count)
    // return CP0_COUNT
    mfc0    v0, CP0_COUNT
    jr      ra
END(get_cp0_count)

LEAF(reset_cp0_count)
    // reset CP0_COUNT
    mtc0    zero, CP0_COUNT
    jr      ra
END(reset_cp0_count)

LEAF(set_cp0_compare)
    // set CP0_COMPARE
    mtc0    a0, CP0_COMPARE
    jr      ra
END(set_cp0_compare)
/*
LEAF(set_cp0_entrylo)
    // set CP0_ENTRYLO
    and     a0, ~0x7
    lw      k0, (a0)
    lw      k1, 4(a0)
    mtc0    k0, CP0_ENTRYLO0
    mtc0    k1, CP0_ENTRYLO1

    jr      ra 
END(set_cp0_entrylo)

LEAF(get_cp0_entryhi)
    mfc0    v0, CP0_ENTRYHI
    jr      ra
END(get_cp0_entryhi)

LEAF(set_cp0_entryhi)
    mtc0    a0, CP0_ENTRYHI
    jr      ra
END(set_cp0_entryhi)

LEAF(clear_TLBe)
    mfc0    t2, CP0_ENTRYHI
    
    mtc0    a0, CP0_ENTRYHI
    tlbp    
    mfc0    t0, CP0_INDEX
    srl     t0, t0, 31
    bnez    t0, No_TLBe

    mtc0    zero, CP0_ENTRYHI
    mtc0    zero, CP0_ENTRYLO0
    mtc0    zero, CP0_ENTRYLO1
    tlbwi
    
No_TLBe:
    mtc0    t2, CP0_ENTRYHI
    jr      ra
END(clear_TLBe)

LEAF(init_TLB)
    li      t1, 32
LOOP:
    sub     t1, t1, 1
    mtc0    zero, CP0_ENTRYHI
    mtc0    zero, CP0_ENTRYLO0
    mtc0    zero, CP0_ENTRYLO1
    mtc0    t1, CP0_INDEX
    tlbwi
    bnez    t1, LOOP

    jr      ra
END(init_TLB)
*/