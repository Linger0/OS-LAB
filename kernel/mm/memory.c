#include "mm.h"
#include "sched.h"
#include "screen.h"

#define P2V(a) ((a) | 0xa0000000)

uint32_t initial_ptebase = PGDIR_BASE >> 12;	// 页目录初始化的 ptebase
pde_t initial_pde = {	// 初始化的页目录项
	.c0 = 0,
	.ptebase = PGDIR_BASE >> 12	// 指向一页初始化的页表
};
pte_t initial_pte = {	// 初始化的页表项
	.G = 0,
	.V = 0,
	.D = 1,
	.C = 2,
	.pfn = 0
};

uint32_t page_pointer = PAGE_PHYBASE;	// 分配物理内存指针
uint32_t ptable_pointer = PTABLE_BASE;	// 分配页表指针

// swap
pie_t initial_pie = {
	.pte = 0,
	.asid = 0,
	.c0 = 0,
	.vpn2 = 0,
	.pin = 0
};
int ppage_n = (PAGE_PHYSTOP - PAGE_PHYBASE) / PAGE_SIZE;	// 物理页框数
pie_t pie[(PAGE_PHYSTOP - PAGE_PHYBASE) / PAGE_SIZE];		// 物理页信息表
uint32_t sd_card_pointer = 0x00100000;	// SD卡内存分配指针

void init_page_table()
{
	int i;
	pde_t *pde = (pde_t *)(PGDIR_BASE + PAGE_SIZE);
	pte_t *pte = (pte_t *)PGDIR_BASE;
	// 初始化一页页表
	for (i = 0; i < 1024; i++) {
		pte[i] = initial_pte;
	}
	// 初始化所有页目录: 指向上面初始化的页表
	for (i = 0; i < 16*1024; i++) {
		pde[i] = initial_pde;
	}
}

void init_swap()
{
	int i;
	// 初始化物理页信息表
	for (i = 0; i < ppage_n; i++) {
		pie[i] = initial_pie;
	}
}

void do_TLBinvalid(uint32_t badvaddr)
{
	pde_t *pde;
	pte_t *pte;
	uint32_t pdn = (badvaddr >> 20) & 0xffc;
	uint32_t ptebase;
	uint32_t ptn = (badvaddr >> 10) & 0xffc;
	pde = (pde_t *)(current_running->pde_base | pdn);	// PDE = PDEBase || PDN || 00
	if (pde->ptebase == initial_ptebase) {	// 分配页表
		do_alloc_ptable(pde);
	}
	ptebase = pde->ptebase << 12;
	pte = (pte_t *)(ptebase | ptn);	// PTE = PTEBASE || PTN || 00
	if (pte->pfn == 0) {	// 分配物理内存
		uint32_t pin = (badvaddr >= 0x7ffff000);	// 栈页框
		do_alloc_ppage(pte, pin, badvaddr & ~0x1fff);
	}
	else if (!pte->V) {	// 换入被换出的页
		do_swap_in(pte, badvaddr & ~0x1fff);
	}
	set_cp0_entrylo(pte);	// 设置 CP0_ENTRYLO
}

void do_alloc_ptable(pde_t *pde) 
{
	if (ptable_pointer == PTABLE_MEM_STOP) oom_exit();

	int i;
	pte_t *pte = (pte_t *)ptable_pointer;
	pde->ptebase = ptable_pointer >> 12;
	for (i = 0; i < 1024; i++) {	// 初始化刚分配的页表
		pte[i] = initial_pte;
	}
	ptable_pointer += PAGE_SIZE;
}

void do_alloc_ppage(pte_t *pte, uint32_t pin, uint32_t vpn2)
{
	int n = (page_pointer - PAGE_PHYBASE) / PAGE_SIZE;	// pie表索引号
	while (pie[n].asid != 0) {	// 有进程正在使用页框
		if (pie[n].pin) {
			page_pointer += PAGE_SIZE;
			if (page_pointer == PAGE_PHYSTOP) 
				page_pointer = PAGE_PHYBASE;
			n = (page_pointer - PAGE_PHYBASE) / PAGE_SIZE;
		}
		else {
			do_swap_out(&pie[n]);	// 把page_pointer指向的页换出
			break;
		}
	}
	// 分配物理页框
	//bzero(P2V(page_pointer), PAGE_SIZE);
	pte->pfn = page_pointer >> 12;
	pte->V = 1;
	pie[n].pin = pin;
	pie[n].pte = pte;
	pie[n].asid = current_running->pid;
	pie[n].vpn2 = vpn2;

	page_pointer += PAGE_SIZE;
	if (page_pointer == PAGE_PHYSTOP) 
		page_pointer = PAGE_PHYBASE;
}

void do_swap_out(pie_t *_pie) 
{
	pte_t *pte = _pie->pte;
	uint32_t cp0_entryhi = (_pie->vpn2 << 13) | _pie->asid;
	uint8_t *str = (uint8_t *)P2V(pte->pfn << 12);
	sdwrite(str, sd_card_pointer, 8);

	pte->V = 0;
	pte->pfn = sd_card_pointer >> 12;
	sd_card_pointer += PAGE_SIZE;
	clear_TLBe(cp0_entryhi);
}

void do_swap_in(pte_t *pte, uint32_t vpn2)
{
	uint32_t sd_card_pfn = pte->pfn << 12;
	do_alloc_ppage(pte, 0, vpn2);
	uint8_t *str = (uint8_t *)P2V(pte->pfn << 12);

	sdread(str, sd_card_pfn, 8);
}

void oom_exit(void)
{
	printk("[ERROR] Out of memory!");
	do_exit();
}