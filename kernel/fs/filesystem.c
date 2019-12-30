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
	// memcpy(buff, (char *)(offset + 0xa1000000 - FS_ADDR), size);
	sdread(buff, offset, size);
}

static void mysdwrite(char *buff, uint32_t offset, uint32_t size)
{
	// memcpy((char *)(offset + 0xa1000000 - FS_ADDR), buff, size);
	sdwrite(buff, offset, size);
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
	bzero(fdesc_table, FDESC_NUM * sizeof(fdesc_t));	// 描述符
	mysdread((char *)sb, FS_ADDR, 512);
	if (sb->magic == MAGIC) {
		imap_addr = FS_ADDR + I2A(1);
		bmap_addr = FS_ADDR + I2A(4);
		inode_addr = sb->inode_addr;
		block_addr = sb->block_addr;
		printk("> [INIT] Filesystem existed. Magic : 0x%x\n", MAGIC);
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
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

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
			mysdwrite((char *)imap, imap_addr, 512);	// imap
			break;
		}
	}
	mysdwrite((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);	// 父目录
	// bno
	mysdread((char *)bmap, bmap_addr, 512*64); // size
	for (i = 0; i < BLOCK_MAX_NUM; i++) {
		if (!bit_read(bmap, i)) {
			bno = i;
			bit_write(bmap, i, 1);
			mysdwrite((char *)bmap, bmap_addr, 512*64);
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
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

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
	strcpy(dir[i].fname, dir[denum - 1].fname);
	dir[i].ino = dir[denum - 1].ino;
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
	if (*dirname == '\0') return;
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

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
		if (i == denum) { printkn("%s: No such directory.\n", path[index]); return; }
	}
	if (inode[INO2I(ino)].type != DIRECTORY) { printkn("%s is not a directory.\n", path[level - 1]); return; }
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
	if (*dirname == '\0') return;
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

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
	// 进入目录
	if (inode[INO2I(ino)].type != DIRECTORY) { printkn("%s is not a directory\n", path[level - 1]); return;}
	current_running->cwd = ino;
	int n = strlen(cwd_path);
	if (!strcmp(dirname, "..")) {
		for (n-- ; n > 0; n--) {
			if (cwd_path[n] == '/') {
				cwd_path[n] = '\0'; break;
			}
			cwd_path[n] = '\0';
		}
	} else if (*dirname == '/') {
		strcpy(cwd_path, dirname);
	} else {
		if (n > 1) cwd_path[n++] = '/';
		strcpy(cwd_path + n, dirname);
	}
}

void do_mknod(char *fname)
{
    if (*fname == '\0') return;

	uint32_t ino, bno;
	uint32_t denum;
	inode_t *inode = (inode_t *)FS_BUFF;
	dentry_t *dir = (dentry_t *)(FS_BUFF + 512);
	// 找当前目录
	ino = current_running->cwd;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	bno = inode[INO2I(ino)].block[0];
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	// 创建文件 inode
	int i;
	uint32_t *imap = (uint32_t *)(FS_BUFF + 0x2000);
	denum = inode[INO2I(ino)].size / sizeof(dentry_t);
	inode[INO2I(ino)].size += sizeof(dentry_t);
	inode[INO2I(ino)].mtime = get_timer();
	mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);	// 目录 inode
	strcpy(dir[denum].fname, fname);
	mysdread((char *)imap, imap_addr, 512);
	for (i = 0; i < INODE_MAX_NUM; i++) {
		if (!bit_read(imap, i)) {
			dir[denum].ino = ino = i;
			bit_write(imap, i, 1);
			mysdwrite((char *)imap, imap_addr, 512);	// imap
			break;
		}
	}
	mysdwrite((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);	// 目录
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	inode[INO2I(ino)].type = NORMAL;
	inode[INO2I(ino)].w = 1;
	inode[INO2I(ino)].r = 1;
	inode[INO2I(ino)].ctime = inode[INO2I(ino)].mtime = get_timer();
	inode[INO2I(ino)].size = 0;
	inode[INO2I(ino)].reference = 1;
	inode[INO2I(ino)].block[0] = 0;
	mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);
}

void do_cat(char *fname)
{
    if (*fname == '\0') return;
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

	uint32_t ino, bno;
	uint32_t denum;
	inode_t *inode = (inode_t *)FS_BUFF;
	dentry_t *dir = (dentry_t *)(FS_BUFF + 512);
	// 找当前目录
	ino = current_running->cwd;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	bno = inode[INO2I(ino)].block[0];
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	// 找文件 inode
	int i;
	denum = inode[INO2I(ino)].size / sizeof(dentry_t);
	for (i = 0; i < denum; i++) {
		if (!strcmp(dir[i].fname, fname)) {
			ino = dir[i].ino;
			break;
		}
	}
	if (i == denum) { printkn("%s: No such file\n", fname); return; }
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	// 输出
	if (inode[INO2I(ino)].type != NORMAL) { printkn("%s is a directory.\n", fname); return; }
	uint32_t size = inode[INO2I(ino)].size;
	int bnum = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
	if (bnum > I_BLOCK_NUM) bnum = I_BLOCK_NUM;
	size = inode[INO2I(ino)].size;
	char *block = (char *)(FS_BUFF + 0x2000);
	// 小块
	for (i = 0; i < bnum; i++) {
		int j;
		uint32_t tmpsize = BLOCK_SIZE;
		if (tmpsize > size) tmpsize = size;
		bno = inode[INO2I(ino)].block[i];
		mysdread(block, block_addr + BNO2A(bno), BLOCK_SIZE);
		for (j = 0; j < tmpsize; j++) {
			printkn("%c", block[j]);
		}
		size -= tmpsize;
	} 
	// 大块
	if (size == 0) return;
	uint32_t *chunk_t = (uint32_t *)(FS_BUFF + 0x1000);
	bno = inode[INO2I(ino)].chunk_t1;
	mysdread((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	int chnum = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
	if (chnum > 1024) chnum = 1024;
	for (i = 0; i < chnum; i++) {
		int j;
		uint32_t tmpsize = CHUNK_SIZE;
		if (tmpsize > size) tmpsize = size;
		bno = chunk_t[i];
		mysdread(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		for (j = 0; j < tmpsize; j++) {
			printkn("%c", block[j]);
		}
		size -= tmpsize;
	}
	// 大块
	if (size == 0) return;
	bno = inode[INO2I(ino)].chunk_t2;
	mysdread((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	chnum = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;
	if (chnum > 1024) chnum = 1024;
	for (i = 0; i < chnum; i++) {
		int j;
		uint32_t tmpsize = CHUNK_SIZE;
		if (tmpsize > size) tmpsize = size;
		bno = chunk_t[i];
		mysdread(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		for (j = 0; j < tmpsize; j++) {
			printkn("%c", block[j]);
		}
		size -= tmpsize;
	}
}

int do_fopen(char *fname, uint32_t access)
{
    if (*fname == '\0') return -1;
	vt100_move_cursor(screen_cursor_x, screen_cursor_y);

	uint32_t ino, bno;
	uint32_t denum;
	inode_t *inode = (inode_t *)FS_BUFF;
	dentry_t *dir = (dentry_t *)(FS_BUFF + 512);
	// 找当前目录
	ino = current_running->cwd;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	bno = inode[INO2I(ino)].block[0];
	mysdread((char *)dir, block_addr + BNO2A(bno), BLOCK_SIZE);
	// 找文件 ino
	int i;
	denum = inode[INO2I(ino)].size / sizeof(dentry_t);
	for (i = 0; i < denum; i++) {
		if (!strcmp(dir[i].fname, fname)) {
			ino = dir[i].ino;
			break;
		}
	}
	if (i == denum) { printkn("%s: No such file\n", fname); return -1; }
	// 创建文件描述符
	for (i = 0; i < FDESC_NUM && fdesc_table[i].ino; i++);
	fdesc_table[i].ino = ino;
	fdesc_table[i].r_offset = 0;
	fdesc_table[i].w_offset = 0;
	fdesc_table[i].wr = access & 0x1;
	fdesc_table[i].rd = access >> 1;
	return i;
}

int do_fread(int fd, char *buff, int size)
{
	if (fd < 0 || !fdesc_table[fd].rd) return 0;
	int bnum;
	inode_t *inode = (inode_t *)FS_BUFF;
	char *block = (char *)(FS_BUFF + 0x2000);
    uint32_t ino = fdesc_table[fd].ino;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	if (inode[INO2I(ino)].type != NORMAL) return 0; 

	int bcnt;	// 文件块计数
	uint32_t bno;
	uint32_t poffset = fdesc_table[fd].r_offset;
	bcnt = poffset / BLOCK_SIZE;
	bnum = (inode[INO2I(ino)].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
	if (bnum > I_BLOCK_NUM) bnum = I_BLOCK_NUM;
	// 小块
	for ( ; bcnt < bnum && size > 0; bcnt++) {
		uint32_t tmpsize = BLOCK_SIZE - (poffset % BLOCK_SIZE);
		if (tmpsize > size) tmpsize = size;
		bno = inode[INO2I(ino)].block[bcnt];
		mysdread(block, block_addr + BNO2A(bno), BLOCK_SIZE);
		memcpy(buff, block + (poffset % BLOCK_SIZE), tmpsize);
		poffset += tmpsize; buff += tmpsize;
		size -= tmpsize;
	}
	fdesc_table[fd].r_offset = poffset;
	// 大块
	if (size == 0) return 1;
	uint32_t chcnt = (poffset - I_BLOCK_NUM * BLOCK_SIZE) / CHUNK_SIZE;
	uint32_t *chunk_t = (uint32_t *)(FS_BUFF + 0x1000);
	bno = inode[INO2I(ino)].chunk_t1;
	mysdread((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	for ( ; chcnt < 1024 && size > 0; chcnt++) {
		uint32_t tmpsize = CHUNK_SIZE - (poffset % CHUNK_SIZE);
		if (tmpsize > size) tmpsize = size;
		bno = chunk_t[chcnt];
		mysdread(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		memcpy(buff, block + (poffset % BLOCK_SIZE), tmpsize);
		poffset += tmpsize; buff += tmpsize;
		size -= tmpsize;
	}	
	fdesc_table[fd].r_offset = poffset;
	// 大块
	if (size == 0) return 1;
	chcnt = (poffset - I_BLOCK_NUM * BLOCK_SIZE - CHUNK_SIZE) / CHUNK_SIZE - 1024;
	bno = inode[INO2I(ino)].chunk_t2;
	mysdread((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	for ( ; chcnt < 1024 && size > 0; chcnt++) {
		uint32_t tmpsize = CHUNK_SIZE - (poffset % CHUNK_SIZE);
		if (tmpsize > size) tmpsize = size;
		bno = chunk_t[chcnt];
		mysdread(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		memcpy(buff, block + (poffset % BLOCK_SIZE), tmpsize);
		poffset += tmpsize; buff += tmpsize;
		size -= tmpsize;
	}
	fdesc_table[fd].r_offset = poffset;
	return 1;
}

int do_fwrite(int fd, char *data, int size)
{
	if (fd < 0 || !fdesc_table[fd].rd) return 0;

	int bnum;
	inode_t *inode = (inode_t *)FS_BUFF;
	char *block = (char *)(FS_BUFF + 0x2000);
    uint32_t ino = fdesc_table[fd].ino;
	mysdread((char *)inode, inode_addr + INO2A(ino), 512);
	if (inode[INO2I(ino)].type != NORMAL) return 0; 

	int bcnt;	// 文件块计数
	uint32_t bno;
	uint32_t poffset = fdesc_table[fd].w_offset;
	uint32_t *bmap = (uint32_t *)(FS_BUFF + 0x40000);
	int index = 0;
	bcnt = poffset / BLOCK_SIZE;
	bnum = (inode[INO2I(ino)].size + BLOCK_SIZE - 1) / BLOCK_SIZE; 
	mysdread((char *)bmap, bmap_addr, 512*64);
	// 小块
	for ( ; bcnt < I_BLOCK_NUM && size > 0; bcnt++) {
		uint32_t tmpsize = BLOCK_SIZE - (poffset % BLOCK_SIZE);
		if (tmpsize > size) tmpsize = size;
		if (bcnt >= bnum) {	// 分配数据块
			for ( ; index < BLOCK_MAX_NUM; index++) {
				if (!bit_read(bmap, index)) {
					bno = index;
					bit_write(bmap, index, 1);
					break;
				}
			}
			inode[INO2I(ino)].block[bcnt] = bno;
			bnum++;
		} else {
			bno = inode[INO2I(ino)].block[bcnt];
			mysdread(block, block_addr + BNO2A(bno), BLOCK_SIZE);
		}
		if (inode[INO2I(ino)].size < poffset + tmpsize) inode[INO2I(ino)].size = poffset + tmpsize;
		memcpy(block + (poffset % BLOCK_SIZE), data, tmpsize);
		mysdwrite(block, block_addr + BNO2A(bno), BLOCK_SIZE);
		poffset += tmpsize; data += tmpsize;
		size -= tmpsize;
	}
	// 大块
	if (size == 0) goto end;
	uint32_t chcnt = (poffset - I_BLOCK_NUM * BLOCK_SIZE) / CHUNK_SIZE;
	uint32_t *chunk_t = (uint32_t *)(FS_BUFF + 0x1000);
	uint32_t chnum = (inode[INO2I(ino)].size + CHUNK_SIZE - 1 - I_BLOCK_NUM * BLOCK_SIZE) / CHUNK_SIZE; 
	if (chnum == 0) {	// 分配 chunk table
		int i;
		for (i = 0 ; i < BLOCK_MAX_NUM; i++) {
			if (!bit_read(bmap, i)) {
				bno = i;
				bit_write(bmap, i, 1);
				inode[INO2I(ino)].chunk_t1 = bno;
				break;
			}
		}
	} else {
		bno = inode[INO2I(ino)].chunk_t1;
		mysdread((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	}
	index = 0;
	for ( ; chcnt < 1024 && size > 0; chcnt++) {
		uint32_t tmpsize = CHUNK_SIZE - (poffset % CHUNK_SIZE);
		if (tmpsize > size) tmpsize = size;
		if (chcnt >= chnum) {	// 分配数据块
			for ( ; index < BLOCK_MAX_NUM / 32; index++) {
				if (bmap[index] == 0x0) {
					bno = index * 32;
					bmap[index] = 0xffffffff;
					break;
				}
			}
			chunk_t[chcnt] = bno;
			chnum++;
		} else {
			bno = chunk_t[chcnt];
			mysdread(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		}
		if (inode[INO2I(ino)].size < poffset + tmpsize) inode[INO2I(ino)].size = poffset + tmpsize;
		memcpy(block + (poffset % BLOCK_SIZE), data, tmpsize);
		mysdwrite(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		poffset += tmpsize; data += tmpsize;
		size -= tmpsize;
	}
	bno = inode[INO2I(ino)].chunk_t1;
	mysdwrite((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	// 大块
	if (size == 0) goto end;
	chcnt = (poffset - I_BLOCK_NUM * BLOCK_SIZE) / CHUNK_SIZE - 1024;
	chunk_t = (uint32_t *)(FS_BUFF + 0x1000);
	chnum = (inode[INO2I(ino)].size + CHUNK_SIZE - 1 - I_BLOCK_NUM * BLOCK_SIZE) / CHUNK_SIZE - 1024; 
	if (chnum == 0) {	// 分配 chunk table
		int i;
		for (i = 0 ; i < BLOCK_MAX_NUM; i++) {
			if (!bit_read(bmap, i)) {
				bno = i;
				bit_write(bmap, i, 1);
				inode[INO2I(ino)].chunk_t1 = bno;
				break;
			}
		}
	} else {
		bno = inode[INO2I(ino)].chunk_t2;
		mysdread((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
	}
	for ( ; chcnt < 1024 && size > 0; chcnt++) {
		uint32_t tmpsize = CHUNK_SIZE - (poffset % CHUNK_SIZE);
		if (tmpsize > size) tmpsize = size;
		if (chcnt >= chnum) {	// 分配数据块
			for ( ; index < BLOCK_MAX_NUM / 32; index++) {
				if (bmap[index] == 0) {
					bno = index * 32;
					bmap[index] = 0xffffffff;
					break;
				}
			}
			chunk_t[chcnt] = bno;
			chnum++;
		} else {
			bno = chunk_t[chcnt];
			mysdread(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		}
		if (inode[INO2I(ino)].size < poffset + tmpsize) inode[INO2I(ino)].size = poffset + tmpsize;
		memcpy(block + (poffset % BLOCK_SIZE), data, tmpsize);
		mysdwrite(block, block_addr + BNO2A(bno), CHUNK_SIZE);
		poffset += tmpsize; data += tmpsize;
		size -= tmpsize;
	}
	bno = inode[INO2I(ino)].chunk_t2;
	mysdwrite((char *)chunk_t, block_addr + BNO2A(bno), BLOCK_SIZE);
end:
	inode[INO2I(ino)].mtime = get_timer();
	mysdwrite((char *)bmap, bmap_addr, 512*64);
	mysdwrite((char *)inode, inode_addr + INO2A(ino), 512);
	fdesc_table[fd].w_offset = poffset;
	return 1;	
}

void do_fclose(int fd)
{
	if (fd < 0) return;
    fdesc_table[fd].ino = 0;
}

int do_fseek(int fd, int pos)
{
	if (fd < 0) return 0;
	fdesc_table[fd].r_offset = pos;
	return 1;
}