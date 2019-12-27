#include "fs.h"
#include "screen.h"
#include "sched.h"
#include "string.h"
#include "test.h"

fdesc_t fdesc_table[FDESC_NUM];

uint32_t imap_addr;
uint32_t bmap_addr;
uint32_t inode_addr;
uint32_t block_addr;

static void mysdread(char *buff, uint32_t offset, uint32_t size)
{
	memcpy(buff, (char *)(offset + 0xa1000000 - FS_ADDR), size);
}

static void mysdwrite(char *buff, uint32_t offset, uint32_t size)
{
	memcpy((char *)(offset + 0xa1000000 - FS_ADDR), buff, size);
}

static uint32_t bit_read(uint32_t *start, uint32_t offset)
{
	uint32_t wordn = offset / 32;
	uint32_t bitn = offset % 32;
	return ((start[wordn] & (1 << bitn)) != 0x0);
}

static void bit_write(uint32_t *start, uint32_t offset, uint32_t bit)
{
	uint32_t wordn = offset / 32;
	uint32_t bitn = offset % 32;
	if (bit) {
		start[wordn] |= (1 << bitn);
	}
	else {
		start[wordn] &= ~(1 << bitn);
	}
}

static int translate_path(char *path_str, char path_array[DIR_LEVEL][FNAME_LEN])
{
	int level = 0;
	int i = 0;
	if (*path_str == '/') {
		*(path_array[level]) = '\0';
		path_str++; level++;
	}
	while (*path_str) {
		if (*path_str == '/') {
			path_array[level][i] = '\0';
			i = 0; level++; path_str++;
		} else {
			path_array[level][i++] = *path_str;
			path_str++;
		}
	}
	if (i != 0) {
		path_array[level][i] = '\0';
		level++;		
	}
	return level;
}

void init_fs()
{
	superblock_t *sb = (superblock_t *)FS_BUFF;
	mysdread((char *)sb, FS_ADDR, 512);
	if (sb->magic == MAGIC) {
		printk("[INIT] Filesystem existed. Magic : 0x%x\n", MAGIC);
	} else {
		screen_cursor_y = 1;
		do_mkfs();
	}
}

void do_mkfs(void)
{
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

	superblock_t *sb;
	uint32_t *imap;
	uint32_t *bmap;
	inode_t *root_inode;
	dentry_t *root_dir;

	printkn("[FS] Start initialize filesystem!\n");
	sb = (superblock_t *)FS_BUFF;
	bzero((char *)sb, 512);
	// super block
	printkn("[FS] Setting superblock...\n");
	sb->magic = MAGIC;
	printkn("     magic : 0x%x\n", MAGIC);
	sb->inode_addr = inode_addr = FS_ADDR + I2A(68);
	sb->inode_num = INODE_MAX_NUM;
	sb->block_addr = block_addr = FS_ADDR + I2A(580);
	sb->block_num = BLOCK_MAX_NUM;
	sb->sectors = BLOCK_MAX_NUM * 8 + 580;
	mysdwrite((char *)sb, FS_ADDR, 512);
	mysdwrite((char *)sb, (FS_ADDR + 1024), 512);	// backup
	printkn("     num sector : %d, start sector : %d\n", sb->sectors, FS_ADDR / 512);
	printkn("     inode map offset : 1 (1)\n");
	printkn("     block map offset : 4 (64)\n");
	printkn("     inode offset : 68 (512)\n");
	printkn("     data offset : 580 (2097152)\n");
	printkn("     inode entry size : %dB, dir entry size : %dB\n", sizeof(inode_t), sizeof(dentry_t));
	// inode map
	printkn("[FS] Setting inode-map...\n");
	imap = (uint32_t *)FS_BUFF;
	bzero((char *)imap, 512);
	bit_write(imap, 0, 1);
	imap_addr = FS_ADDR + I2A(1);
	mysdwrite((char *)imap, imap_addr, 512);
	// block map
	printkn("[FS] Setting block-map...\n");
	bmap = (uint32_t *)FS_BUFF;
	bzero((char *)bmap, 512*64);
	bit_write(bmap, 0, 1);
	bmap_addr = FS_ADDR + I2A(4);
	mysdwrite((char *)bmap, bmap_addr, 64*512);
	// root dir inode
	printkn("[FS] Setting inode...\n");
	root_inode = (inode_t *)FS_BUFF;
	bzero((char *)root_inode, 512);
	root_inode->type = DIRECTORY;
	root_inode->w = 1;
	root_inode->r = 1;
	root_inode->ctime = root_inode->mtime = get_timer();
	root_inode->size = 2 * sizeof(dentry_t);
	root_inode->reference = 1;
	root_inode->block[0] = 0;
	mysdwrite((char *)root_inode, inode_addr, 512);
	// root directory
	root_dir = (dentry_t *)FS_BUFF;
	bzero((char *)root_dir, 512);
	strcpy(root_dir[0].fname, ".");
	root_dir[0].ino = 0;
	strcpy(root_dir[1].fname, "..");
	root_dir[1].ino = 0;
	mysdwrite((char *)root_dir, block_addr, 512);
	printkn("[FS] Initialize filesystem finished!\n");
}

void do_fs_info()
{
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

	superblock_t *sb = (superblock_t *)FS_BUFF;
	uint32_t *imap;
	uint32_t *bmap;
	int used_inode = 0, used_blk = 0;
	mysdread((char *)sb, FS_ADDR, 512);
	printkn("     magic : 0x%x\n", sb->magic);
	printkn("     num sector : %d, start sector : %d\n", sb->sectors, FS_ADDR / 512);

	int i;
	imap = (uint32_t *)FS_BUFF;
	mysdread((char *)imap, imap_addr, 512);
	for (i = 0; i < INODE_MAX_NUM; i++) {
		if (bit_read(imap, i)) used_inode++;
	}

	int j;
	bmap = (uint32_t *)FS_BUFF;
	for (j = 0; j < 64; j++) {
		mysdread((char *)bmap, bmap_addr + I2A(j), 512);
		for (i = 0; i < 4096; i++) {
			if (bit_read(bmap, i)) used_blk++;
		}
	}
	printkn("     inode map offset : 1 (1), used : %d/%d\n", used_inode, INODE_MAX_NUM);
	printkn("     block map offset : 4 (64), used : %d/%d\n", used_blk, BLOCK_MAX_NUM);
	printkn("     inode offset : 68 (512)\n");
	printkn("     data offset : 580 (2097152)\n");
	printkn("     inode entry size : %dB, dir entry size : %dB\n", sizeof(inode_t), sizeof(dentry_t));	
}

void do_mkdir(char *dirname)
{
	if (*dirname == '\0') return;

	int level, index;
	uint32_t ino, bno;
	uint32_t denum;
	inode_t *inode = (inode_t *)FS_BUFF;
	dentry_t *dir = (dentry_t *)(FS_BUFF + 512);
	char path[DIR_LEVEL][FNAME_LEN];
	level = translate_path(dirname, path);
	// 找父目录
	if (path[0][0] == '\0') {
		ino = 0;
		mysdread((char *)inode, inode_addr + INO2A(ino), 512);
		bno = 0;
		index = 1;
	} else {
		ino = current_running->cwd;
		mysdread((char *)inode, inode_addr + INO2A(ino), 512);
		bno = inode[INO2I(ino)].block[0];
		index = 0;
	}
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	while (index < level - 1) {
		denum = inode[INO2I(ino)].size / sizeof(dentry_t);
		int i;
		for (i = 0; i < denum; i++) {
			if (!strcmp(dir[i].fname, path[index])) {
				ino = dir[i].ino;
				mysdread((char *)inode, inode_addr + INO2A(ino), 512);
				bno = inode[INO2I(ino)].block[0];
				mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
				index++; break;
			}
		}
		if (i == denum) { printkn("%s: No such directory\n", path[index]); return; }
	}
	// 创建目录
	int i;
	uint32_t pino = ino;
	uint32_t *imap = (uint32_t *)(FS_BUFF + 0X2000);
	uint32_t *bmap = (uint32_t *)(FS_BUFF + 0x3000);
	denum = inode[INO2I(ino)].size / sizeof(dentry_t);
	inode[INO2I(ino)].size += sizeof(dentry_t);
	inode[INO2I(ino)].mtime = get_timer();
	inode[INO2I(ino)].reference++;
	mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);	// 父 inode
	strcpy(dir[denum].fname, path[level - 1]); 
	// ino
	mysdread((char *)imap, imap_addr, 512);
	for (i = 0; i < INODE_MAX_NUM; i++) {
		if (!bit_read(imap, i)) {
			dir[denum].ino = ino = i;
			bit_write(imap, i, 1);
			mysdwrite((char *)imap, imap_addr, 512);
			break;
		}
	}
	mysdwrite((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);	// 父目录
	// bno
	mysdread((char *)bmap, bmap_addr, 512); // size
	for (i = 0; i < BLOCK_MAX_NUM; i++) {
		if (!bit_read(bmap, i)) {
			bno = i;
			bit_write(bmap, i, 1);
			mysdwrite((char *)bmap, bmap_addr, 512);
			break;
		}
	}
	// inode
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	inode[INO2I(ino)].type = DIRECTORY;
	inode[INO2I(ino)].w = 1;
	inode[INO2I(ino)].r = 1;
	inode[INO2I(ino)].ctime = inode[INO2I(ino)].mtime = get_timer();
	inode[INO2I(ino)].size = 2 * sizeof(dentry_t);
	inode[INO2I(ino)].reference = 1;
	inode[INO2I(ino)].block[0] = bno;
	mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);
	// block
	bzero((char *)dir, 512);
	strcpy(dir[0].fname, ".");
	dir[0].ino = ino;
	strcpy(dir[1].fname, "..");
	dir[1].ino = pino;
	mysdwrite((char *)dir, block_addr + BNO2A(bno), 512);
}

void do_rmdir(char *dirname)
{
	int level, index;
	uint32_t ino, bno;
	uint32_t denum;
	inode_t *inode = (inode_t *)FS_BUFF;
	dentry_t *dir = (dentry_t *)(FS_BUFF + 512);
	char path[DIR_LEVEL][FNAME_LEN];
	level = translate_path(dirname, path);
	// 找父目录
	if (path[0][0] == '\0') {
		ino = 0;
		mysdread((char *)inode, inode_addr + INO2A(ino), 512);
		bno = 0;
		index = 1;
	} else {
		ino = current_running->cwd;
		mysdread((char *)inode, inode_addr + INO2A(ino), 512);
		bno = inode[INO2I(ino)].block[0];
		index = 0;
	}
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	while (index < level - 1) {
		denum = inode[INO2I(ino)].size / sizeof(dentry_t);
		int i;
		for (i = 0; i < denum; i++) {
			if (!strcmp(dir[i].fname, path[index])) {
				ino = dir[i].ino;
				mysdread((char *)inode, inode_addr + INO2A(ino), 512);
				bno = inode[INO2I(ino)].block[0];
				mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
				index++; break;
			}
		}
		if (i == denum) { printkn("%s: No such directory\n", path[index]); return; }
	}
	// 删除目录	
	int i;
	uint32_t cino;
	uint32_t *imap = (uint32_t *)(FS_BUFF + 0X2000);
	uint32_t *bmap = (uint32_t *)(FS_BUFF + 0x3000);
	denum = inode[INO2I(ino)].size / sizeof(dentry_t);
	for (i = 0; i < denum; i++) {
		if (!strcmp(dir[i].fname, path[level - 1])) break;
	}
	cino = dir[i].ino;
	strcpy(dir[i].fname, dir[denum].fname);
	dir[i].ino = dir[denum].ino;
	mysdwrite((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);	// 父目录
	inode[INO2I(ino)].size -= sizeof(dentry_t);
	inode[INO2I(ino)].reference--;
	inode[INO2I(ino)].mtime = get_timer();
	mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);	// 父 inode
	// inode
	ino = cino;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	inode[INO2I(ino)].reference--;
	if (inode[INO2I(ino)].reference > 0) {	// 硬链接数 > 0
		inode[INO2I(ino)].mtime = get_timer();
		mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);
	} else {
		// block map
		bno = inode[INO2I(ino)].block[0];
		mysdread((char *)bmap, bmap_addr + I2A(bno / 4096), 512);
		bit_write(bmap, bno % 4096, 0);
		mysdwrite((char *)bmap, bmap_addr + I2A(bno / 4096), 512);
		// inode map
		mysdread((char *)imap, imap_addr, 512);
		bit_write(imap, ino, 0);
		mysdwrite((char *)imap, imap_addr, 512);
	}
}

void do_read_dir(char *dirname)
{
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

	if (*dirname == '\0') return;

	int level, index;
	int denum;
	uint32_t ino, bno;
	inode_t *inode = (void *)FS_BUFF;
	dentry_t *dir = (void *)(FS_BUFF + 512);
	char path[DIR_LEVEL][FNAME_LEN];
	level = translate_path(dirname, path);
	// 找目录
	if (path[0][0] == '\0') {
		ino = 0;
		mysdread((char *)inode, inode_addr + INO2A(ino), 512);
		bno = 0;
		index = 1;
	} else {
		ino = current_running->cwd;
		mysdread((char *)inode, inode_addr + INO2A(ino), 512);
		bno = inode[INO2I(ino)].block[0];
		index = 0;
	}
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	while (index < level) {
		int i;
		denum = inode[INO2I(ino)].size / sizeof(dentry_t);
		for (i = 0; i < denum; i++) {
			if (!strcmp(dir[i].fname, path[index])) {
				ino = dir[i].ino;
				mysdread((char *)inode, inode_addr + INO2A(ino), 512);
				bno = inode[INO2I(ino)].block[0];
				mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
				index++; break;
			}
		}
		if (i == denum) { printkn("%s: No such directory\n", path[index]); return; }
	}
	// 输出
	int i;
	denum = inode[INO2I(ino)].size / sizeof(dentry_t);
	for (i = 0; i < denum; i++) {
		printkn("%s ", dir[i].fname);
	}
	printkn("\n");
}

void do_enter_fs(char *dirname)
{
	if (*dirname != '/') return;

	int level, index;
	int denum;
	uint32_t ino, bno;
	inode_t *inode = (void *)FS_BUFF;
	dentry_t *dir = (void *)(FS_BUFF + 512);
	char path[DIR_LEVEL][FNAME_LEN];
	level = translate_path(dirname, path);
	// 找目录
	ino = 0;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	bno = 0; index = 1;
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	while (index < level) {
		int i;
		denum = inode[INO2I(ino)].size / sizeof(dentry_t);
		for (i = 0; i < denum; i++) {
			if (!strcmp(dir[i].fname, path[index])) {
				ino = dir[i].ino;
				mysdread((char *)inode, inode_addr + INO2A(ino), 512);
				bno = inode[INO2I(ino)].block[0];
				mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
				index++; break;
			}
		}
		if (i == denum) { printkn("%s: No such directory\n", path[index]); return; }
	}
	// 进入目录
	if (inode[INO2I(ino)].type != DIRECTORY) { printkn("%s: Not a directory\n", path[level - 1]); return;}
	current_running->cwd = ino;
	strcpy(cwd_path, dirname);
}