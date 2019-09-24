#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *Phdr);
void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int phnum, int kernelsz);
Elf32_Phdr *read_exec_file(FILE *opfile, int *phnum);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr, int phnum_kn, int *kernelsz);
void extent_opt(Elf32_Phdr *bbPhdr, Elf32_Phdr *knPhdr, int kernelsz, uint8_t kernelsec);

Elf32_Phdr *read_exec_file(FILE *opfile, int *phnum)
{
	//读取ELF文件并返回程序头指针和数量phnum
	Elf32_Phdr * Phdr;
	Elf32_Ehdr Ehdr;	
	fread(&Ehdr, sizeof(Elf32_Ehdr), 1, opfile);		//读取文件头
	*phnum = Ehdr.e_phnum;					//获取phnum
	Phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr) * *phnum);
	fread(Phdr, sizeof(Elf32_Phdr), *phnum, opfile);	//读取程序头
	return Phdr;
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr, int phnum_kn, int *kernelsz)
{	//计算kernel的扇区数和大小并返回
	*kernelsz = 0;	//kernel的大小
	int i;
	//依次读取每个程序头，将memsz累加
	for (i = 0; i < phnum_kn; i++)
		*kernelsz += Phdr[i].p_memsz;
	return (*kernelsz - 1) / 512 + 1;	//返回扇区数
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
	char buf[512] = {0};	//数据缓冲区，大小为一个扇区，初始化为0
	fseek(file, (long)phdr->p_offset, SEEK_SET);
	fread(buf, sizeof(char), phdr->p_filesz, file);
	fwrite(buf, sizeof(char), 512, image);
	return;
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int phnum, int kernelsz)
{
	char buf[kernelsz];		//数据缓冲区，大小为kernelsz
	memset(buf, 0, kernelsz);
	char * bufp = buf;		//buf中未占用的第一个字节
	int i;
	//依次读取每个程序头，定位segment并载入buf
	for (i = 0; i < phnum; i++) {
		if (Phdr[i].p_type != PT_LOAD) continue;	//跳过非可加载段
		fseek(knfile, (long)Phdr[i].p_offset, SEEK_SET);
		fread(bufp, sizeof(char), Phdr[i].p_filesz, knfile);	//读取segment并载入buf
		bufp += Phdr[i].p_memsz;			//根据memsz设置bufp
	}
	fwrite(buf, sizeof(char), kernelsz, image);
	return;
}

void record_kernel_size(FILE *image, int kernelsz)
{	//将kernel大小写入bootblock的500位置，将结束段55aa写入末尾
	char s[2] = {0x55, 0xaa};
	fseek(image, (long)500, SEEK_SET);
	fwrite(&kernelsz, sizeof(int), 1, image);
	fseek(image, (long)510, SEEK_SET);
	fwrite(s, sizeof(char), 2, image);
	return;
}

void extent_opt(Elf32_Phdr *bbPhdr, Elf32_Phdr *knPhdr, int kernelsz, uint8_t kernelsec)
{
	printf("bootblock message:\n");
	printf("\tbootblock image memory size is 0x%x\n", bbPhdr->p_memsz);
	printf("\tbootblock image offset is 0x%x\n", bbPhdr->p_offset);
	printf("kernel message:\n");
	printf("\tkernel image memory size is 0x%x\n", kernelsz);
	printf("\tkernel image offset is 0x%x\n", knPhdr[0].p_offset);
	printf("\tkernel sector number is 0x%x\n", kernelsec);
	return;
}


int main(int argc, char *argv[])
{
	//读取命令行参数
	int i = 1;	//指示文件名参数位置
	int out = 0;	//指示是否需要打印(--extend)
	if (argc < 3) {printf("ERROE: lack of file\n"); return 0;}
	if (*argv[1]=='-') {
		out = 1; i = 2;
		if (argc < 4) {printf("ERROR: lack of file\n"); return 0;}
	}

	//打开ELF文件
	FILE * bbfile = fopen(argv[i], "rb");
	FILE * knfile = fopen(argv[i+1], "rb");
	FILE * image;

	Elf32_Phdr *bbPhdr, *knPhdr;	//两个文件的程序头
	int bbphnum, knphnum;		//kernel的程序头个数
	uint8_t kernelsec;		//kernel的sector数
	int kernelsz;			//kernel size

	//读取bootblock和kernel文件，返回Phdr和phnum
	bbPhdr = read_exec_file(bbfile, &bbphnum);
	knPhdr = read_exec_file(knfile, &knphnum);
	kernelsec = count_kernel_sectors(knPhdr, knphnum, &kernelsz);	//计算kernel的sector数

	//写image
	image = fopen("image", "wb");
	write_bootblock(image, bbfile, bbPhdr);
	write_kernel(image, knfile, knPhdr, knphnum, kernelsz);
	record_kernel_size(image, kernelsec * 512);
	
	if (out) extent_opt(bbPhdr, knPhdr, kernelsz, kernelsec);
	return 0;
}
