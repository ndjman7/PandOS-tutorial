#define COL8_000000     0
#define COL8_FF0000     1
#define COL8_00FF00     2
#define COL8_FFFF00     3
#define COL8_0000FF     4
#define COL8_FF00FF     5
#define COL8_00FFFF     6
#define COL8_FFFFFF     7
#define COL8_C6C6C6     8
#define COL8_840000     9
#define COL8_008400     10
#define COL8_848400     11
#define COL8_000084     12
#define COL8_840084     13
#define COL8_008484     14
#define COL8_848484     15

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

/* 같은 소스 파일 내에 있더라도 정의한 후에 사용해야 한다.
   다음과 같이 미리 선언해 둔다. */

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(unsigned char *vram, int x, int y);

struct BOOTINFO {
    char cyls, leds, vmode, reserve;
    short scrnx, scrny;
    unsigned char *vram;
};

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;

    init_palette(); /* 팔레트 설정 */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	for (;;) {
		io_hlt();
	}
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1)
{
    int x, y;
    for (y = y0; y <= y1; y++) {
        for (x = x0; x <= x1; x++)
            vram[y * xsize + x] = c;
    }
    return;
}

void init_palette(void)
{
    static unsigned char table_rgb[16 * 3] = {
        0x00, 0x00, 0x00,   /*  0: 검은 색 */
        0xff, 0x00, 0x00,   /*  1: 밝은 적색 */
        0x00, 0xff, 0x00,   /*  2: 밝은 녹색 */
        0xff, 0xff, 0xff,   /*  3: 밝은 노란색 */
        0x00, 0x00, 0xff,   /*  4: 밝은 청색 */
        0xff, 0x00, 0xff,   /*  5: 밝은 보라색 */
        0x00, 0xff, 0xff,   /*  6: 밝은 청색 */
        0xff, 0xff, 0xff,   /*  7: 흰색 */
        0xc6, 0xc6, 0xc6,   /*  8: 밝은 회색 */
        0x84, 0x00, 0x00,   /*  9: 어두운 적색 */
        0x00, 0x84, 0x00,   /* 10: 어두운 녹색 */
        0x84, 0x84, 0x00,   /* 11: 어두운 노란색 */
        0x00, 0x00, 0x84,   /* 12: 군청색 */
        0x84, 0x00, 0x84,   /* 13: 어두운 보라색 */
        0x00, 0x84, 0x84,   /* 14: 어두운 청색 */
        0x84, 0x84, 0x84,   /* 15: 어두운 회색 */
    };
    set_palette(0, 15, table_rgb);
    return;

    /* static char 명령은 데이터밖에 사용할 수 없지만, DB 명령에 대응된다. */
}

void set_palette(int start, int end, unsigned char *rgb)
{
    int i, eflags;
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x0c38, start);
    for (i = start; i <= end; i++) {
        io_out8(0x03c9, rgb[0] / 4);
        io_out8(0x03c9, rgb[1] / 4);
        io_out8(0x03c9, rgb[2] / 4);
        rgb += 3;
    }
    io_store_eflags(eflags);    /* 인터럽트 허가 플래그를 본래 값으로 되돌린다. */
    return;
}

void init_screen(unsigned char *vram, int xsize, int ysize)
{
    boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize - 1, ysize - 29);
    boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize - 1, ysize - 28);
    boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize - 1, ysize - 27);
    boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize - 1, ysize -  1);

    boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,        ysize - 24);
    boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,        ysize -  4);
    boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,        ysize -  4);
    boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,        ysize -  5);
    boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,        ysize -  3);
    boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,        ysize -  3);

    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
    boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
    boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);
    return;
}
