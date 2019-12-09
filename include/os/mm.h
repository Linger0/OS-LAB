#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

#define TLB_ENTRY_NUMBER 64

/* --------- Physical Memory ----------- *
 * 0x0000000..0x0010000: Exception entry *
 * 0x0010000..0x0110000: PMON code       *
 * 0x0200000..0x0300000: Page table      *
 * 0x0800000..0x0900000: Kernel          *
 * 0x0f00000..0x1000000: Kernel Stack    *
 * 0x1000000..0x2000000: User space      *
 * ------------------------------------- */
#define PGDIR_BASE 0xa0200000
#define PTABLE_BASE 0xa0211000
#define PTABLE_MEM_STOP 0xa0300000
#define PAGE_PHYBASE 0x01000000
#define PAGE_PHYSTOP 0x02000000
#define PAGE_SIZE 0x1000

/* --------- VA ---------
 * |31..22|21..12| 11..0|
 * ----------------------
 * | PDN  | PTN  |offset|
 * ---------------------- */
typedef struct pde 
{
    uint32_t c0 : 12;
    uint32_t ptebase : 20;
} pde_t;

typedef struct pte
{
    uint32_t G : 1;
    uint32_t V : 1;
    uint32_t D : 1;
    uint32_t C : 3;
    uint32_t pfn : 26;
} pte_t;

typedef struct page_info_entry
{
    pte_t *pte;
    // CP0_EntryHi
    uint32_t asid : 8;
    uint32_t c0 : 5;
    uint32_t vpn2 : 18;
    uint32_t pin : 1;
} pie_t;

extern pde_t initial_pde;
extern pte_t initial_pte;

void init_TLB();
void handle_TLBinvalid();
void set_cp0_entrylo(pte_t *);
uint32_t get_cp0_entryhi();
void set_cp0_entryhi(uint32_t);
void clear_TLBe(uint32_t);

void init_page_table();             // 初始化页目录
void init_swap();                   // 初始化物理页框管理表

void do_TLBinvalid(uint32_t);       // TLB Invalid
void do_alloc_ptable(pde_t *);      // 分配页表
void do_alloc_ppage(pte_t *, uint32_t, uint32_t); // 分配物理内存
void do_swap_out(pie_t *);
void do_swap_in(pte_t *, uint32_t);

void oom_exit(void);                // out of memory ERROR

#endif
