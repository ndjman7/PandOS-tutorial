void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

/* 같은 소스 파일 내에 있더라도 정의한 후에 사용해야 한다.
   다음과 같이 미리 선언해 둔다. */

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
	int i; /* 변수 선언.i라고 하는 변수는 32 비트의 정수형 */
    char *p; /* p라는 변수는 BYTE [...]용의 번지 */

    init_palette(); /* 팔레트 설정 */

    p = (char *) 0xa0000; /* 번지 대입 */

	for (i = 0; i <= 0xaffff; i++) {
        p[i] = i & 0x0f;
	}

	for (;;) {
		io_hlt();
	}
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
