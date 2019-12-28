#ifndef INCLUDE_FS_H_
#define INCLUDE_FS_H_

#include "type.h"
#include "time.h"

#define MAGIC 0x77777777
#define FS_ADDR 0x20000000    // 512MB
#define FS_BUFF 0xa0400000    // Buffer

/* ---------------------------------------------------------- *
 * |       |SuperBlock|InodeMap|Reserve|BlockMap|Inode|Block| *
 * ---------------------------------------------------------- *
 * |sectors|     1    |    1   |   2   |   64   | 512 | 2^21| *
 * |   size|    512B  |  512B  |  1KB  |  32KB  |256KB| 1GB | *
 * ---------------------------------------------------------- *
 * | nOfsec|     1    |  4096  |       |  4096  |  8  | .125| *
 * |   size|   512B   |  1bit  |       |  1bit  | 64B | 4KB | *
 * ---------------------------------------------------------- */
#define INODE_MAX_NUM 0x1000    // 2^12
#define BLOCK_MAX_NUM 0x40000   // 2^18 * 4KB = 1GB 
#define BLOCK_SIZE 0x1000       // 4KB
#define I_BLOCK_NUM 10
#define FNAME_LEN 11

#define DIR_LEVEL 4
#define FDESC_NUM 16

#define ROUNDDOWN(a) ((a) & ~0xfff)
#define A2I(a) ((a) >> 9)  // Address to Sector Index
#define I2A(i) ((i) << 9)  // Sector Index to Address
#define INO2A(ino) (((ino) / 8) * 512)
#define INO2I(ino) ((ino) % 8)
#define BNO2A(bno) ((bno) * 4096)

enum FileTypes
{
    NORMAL = 0x1,
    DIRECTORY = 0x2
};

typedef struct superblock
{
    uint32_t sectors;       // 文件系统占据的扇区数
    uint32_t inode_num;     // inode 最大个数
    uint32_t block_num;     // 数据块最大个数
    uint32_t inode_addr;    // inode 在磁盘的起始地址
    uint32_t block_addr;    // 数据块在磁盘的起始地址
    uint32_t magic;         // 魔数
} superblock_t;

typedef struct inode
{
    uint32_t type : 16;    // mode: 文件类型
    uint32_t w : 1;        //       write 权限
    uint32_t r : 1;        //       read 权限
    uint32_t reference;          // 硬链接数
    uint32_t ctime;              // 创建时间
    uint32_t mtime;              // 最近修改时间
    uint32_t size;               // 文件大小
    uint32_t block[I_BLOCK_NUM]; // 数据块表
    uint32_t chunk_table;        // 一级间接索引表
} inode_t; /* (6 + 10) * 4B = 64B */

typedef struct dentry
{
    char fname[FNAME_LEN + 1];
    uint32_t ino;
} dentry_t;

typedef struct filedesc
{
    uint32_t ino;
    uint32_t wr : 1;    // 请求权限
    uint32_t rd : 1;
    uint32_t r_offset;  // 读指针
    uint32_t w_offset;  // 写指针
} fdesc_t;

extern char cwd_path[DIR_LEVEL * (FNAME_LEN + 1)];

void init_fs();

void do_mkfs();
void do_fs_info();
void do_mkdir(char *);
void do_rmdir(char *);
void do_read_dir(char *);
void do_enter_fs(char *);

void do_mknod(char *);
void do_cat(char *);
int do_fopen(char *, uint32_t);
int do_fread(int, char *, int);
int do_fwrite(int, char *, int);
void do_fclose(int);

#endif