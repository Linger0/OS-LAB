#include "mm.h"
#include "sched.h"

//TODO:Finish memory management functions here refer to mm.h and add any functions you need.

uint32_t initial_pgd = PAGE_TABLE_BASE >> 12; // 初始化的二级页表 PGD
pte1_t initial_pte1 = {	// 初始化的一级页表项
	.c0 = 0,
	.pgd = PAGE_TABLE_BASE >> 12
};
pte2_t initial_pte2 = {	// 初始化的二级页表项
	.G = 0,
	.V = 0,
	.D = 1,
	.C = 2,
	.pfn = 0,
	.c0 = 0
};

uint32_t ptable2_base = 0xa0111000;	// 分配二级页表基址
uint32_t page_base = 0x01000000;	// 分配物理内存基址

void init_page_table()
{
	int i;
	pte1_t *pte1 = (pte1_t *)(PAGE_TABLE_BASE + PAGE_TABLE_SIZE);
	pte2_t *pte2 = (pte2_t *)PAGE_TABLE_BASE;
	// 初始化一页二级页表
	for (i = 0; i < 1024; i++) {
		pte2[i] = initial_pte2;
	}
	// 初始化所有一级页表: 指向上面初始化的二级页表
	for (i = 0; i < 16*1024; i++) {
		pte1[i] = initial_pte1;
	}
}

void do_TLBinvalid(uint32_t badvaddr)
{
	pte1_t *pte1;
	pte2_t *pte2;
	uint32_t vpn1 = (badvaddr >> 20) & 0xffc;
	uint32_t pgd;
	uint32_t vpn2 = (badvaddr >> 10) & 0xff8; // vpn2 末位为0
	pte1 = (pte1_t *)(current_running->pte1_base | vpn1); // PTE1 = PTE1Base || VPN1 || 00
	if (pte1->pgd == initial_pgd) {	// 分配二级页表
		do_alloc_ptable2(pte1);
	}
	pgd = pte1->pgd << 12;
	pte2 = (pte2_t *)(pgd | vpn2);	// PTE2 = PGD || VPN2 || 00
	if (!pte2->V) do_alloc_page(pte2);	// 分配物理内存
	set_cp0_entrylo(pte2);	// 设置 CP0_ENTRY
}

void do_alloc_ptable2(pte1_t *pte1) 
{
	int i;
	pte2_t *pte2 = (pte2_t *)ptable2_base;
	pte1->pgd = ptable2_base >> 12;
	for (i = 0; i < 1024; i++) {	// 初始化刚分配的二级页表
		pte2[i] = initial_pte2;
	}
	ptable2_base += PAGE_TABLE_SIZE;
}

void do_alloc_page(pte2_t *pte2)
{
	pte2->pfn = page_base >> 12;
	pte2->V = 1;
	page_base += PAGE_SIZE;
	(pte2 + 1)->pfn = page_base >> 12;
	(pte2 + 1)->V = 1;
	page_base += PAGE_SIZE;
}