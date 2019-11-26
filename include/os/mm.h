#ifndef INCLUDE_MM_H_
#define INCLUDE_MM_H_

#include "type.h"

#define TLB_ENTRY_NUMBER 64

#define PAGE_TABLE_BASE 0xa0100000 // MAX: 0xa0111000
#define PAGE_TABLE_SIZE 0x1000
#define PAGE_SIZE 0x1000
/* --------- Physical Memory ------------ *
 * 0x00000000~0x00100000: Exception entry *
 * 0x00800000~0x00900000: Kernel          *
 * 0x00f00000~0x01000000: Stack           *
 * -------------------------------------- */
/* ---------- VA ------
 * |31~22|21~12| 11~0 |
 * --------------------
 * | VPN1| VPN2|offset|
 * -------------------- */

typedef struct pte1 
{
    uint32_t c0 : 12;
    uint32_t pgd : 20;
} pte1_t;

typedef struct pte2
{
    uint32_t G : 1;
    uint32_t V : 1;
    uint32_t D : 1;
    uint32_t C : 3;
    uint32_t pfn : 13;
    uint32_t c0 : 13;
} pte2_t;

extern pte1_t initial_pte1;
extern pte2_t initial_pte2;

void handle_TLBinvalid();
void set_cp0_entrylo(pte2_t *);

void init_page_table();             // 初始化一级页表
void do_TLBinvalid(uint32_t);       // TLB Invalid
void do_alloc_ptable2(pte1_t *);    // 分配二级页表
void do_alloc_page(pte2_t *);       // 分配物理内存

#endif
