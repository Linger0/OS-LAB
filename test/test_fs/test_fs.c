#include "fs.h"
#include "stdio.h"
#include "string.h"
#include "test_fs.h"
#include "syscall.h"

static char buff[64];

void test_fs(void)
{
    int i, j;
    int fd = sys_fopen("1.txt", O_RDWR);

    sys_move_cursor(0, 0);
    for (i = 0; i < 10; i++)
    {
        sys_fwrite(fd, "hello world!\n", 13);
    }

    for (i = 0; i < 10; i++)
    {
        sys_fread(fd, buff, 13);
        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_fclose(fd);
    sys_exit();
}

static char buf[4096] = "It's the beginning of a block!\n";
void test_fs2(void)
{
    int i, j;
    int fd = sys_fopen("2.txt", O_RDWR);

    sys_move_cursor(0, 0);
    for (i = 0; i < 16; i++)
    {
        sys_fwrite(fd, buf, 4096);
        if (i % 2) printf("ok: %d\n", i);
    }

    sys_fseek(fd, 15*4096);
    sys_fread(fd, buff, 64);
    for (j = 0; j < 64; j++)
    {
        printf("%c", buff[j]);
    }

    sys_fclose(fd);
    sys_exit();
}

int rd_off = 0;
void fs_read(void)
{
    int i, j;
    int fd = sys_fopen("2.txt", O_RDWR);

    sys_move_cursor(0, 0);

    sys_fseek(fd, rd_off);
    sys_fread(fd, buff, 64);
    for (j = 0; j < 64; j++)
    {
        printf("%c", buff[j]);
    }

    sys_fclose(fd);
    sys_exit();
}