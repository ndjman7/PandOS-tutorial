#include "bootpack.h"
#include <stdio.h>

#define MEMMAN_FREES        4090            /* 이것으로 약 32KB의 관리 영역이 필요 */
#define MEMMAN_ADDR         0x003c0000

struct FREEINFO {   /* 빈 정보 */
    unsigned int addr, size;
};

struct MEMMAN {     /* 메모리 관리 */
    int frees, maxfrees, lostsize, losts;
    struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

void HariMain(void)
{
    struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
    char s[40], mcursor[256], keybuf[32], mousebuf[128];
    int mx, my, i;
    unsigned int memtotal;
    struct MOUSE_DEC mdec;
    struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

    init_gdtidt();
    init_pic();
    io_sti(); /* IDC/PIC의 초기화가 끝났으므로 CPI의 인터럽트 금지를 해제 */
    fifo8_init(&keyfifo, 32, keybuf);
    fifo8_init(&mousefifo, 128, mousebuf);
    io_out8(PIC0_IMR, 0xf9);
    io_out8(PIC1_IMR, 0xef);

    init_keyboard();
    enable_mouse(&mdec);
    memtotal = memtest(0x00400000, 0xbfffffff);
    memman_init(memman);
    memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
    memman_free(memman, 0x00400000, memtotal - 0x00400000);

    init_palette(); /* 팔레트 설정 */
    init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
    mx = (binfo->scrnx - 16) / 2; /* 화면 중아이 되도록 좌표 계산 */
    my = (binfo->scrny - 28 - 16) / 2;
    init_mouse_cursor8(mcursor, COL8_008484);
    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
    sprintf(s, "(%d, %d)", mx, my);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

    sprintf(s, "memory %dMB   free : %dKB",
            memtotal / (1024 * 1024), memman_total(memman) / 1024);
    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	for (;;) {
		io_cli();
        if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
            io_stihlt();
        } else {
            if (fifo8_status(&keyfifo) != 0) {
                i = fifo8_get(&keyfifo);
                io_sti();
                sprintf(s, "%02X", i);
                boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
                putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
            } else if (fifo8_status(&mousefifo) != 0) {
                i = fifo8_get(&mousefifo);
                io_sti();
                if (mouse_decode(&mdec, i) != 0) {
                    /* 데이터가 3바이트 쌓였으므로 표시 */
                    sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
                    if ((mdec.btn & 0x01) != 0) {
                        s[1] = 'L';
                    }
                    if ((mdec.btn & 0x02) != 0) {
                        s[3] = 'R';
                    }
                    if ((mdec.btn & 0x04) != 0) {
                        s[2] = 'C';
                    }
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
                    putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
                    /* 마우스 커서의 이동 */
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 마우스 없앰 */
                    mx += mdec.x;
                    my += mdec.y;
                    if (mx < 0) {
                        mx = 0;
                    }
                    if (my < 0) {
                        my = 0;
                    }
                    if (mx > binfo->scrnx - 16) {
                        mx = binfo->scrnx - 16;
                    }
                    if (my > binfo->scrny - 16) {
                        my = binfo->scrny - 16;
                    }
                    sprintf(s, "(%3d, %3d)", mx, my);
                    boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
                    /* 좌표 없앰 */
                    putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
                    /* 좌표쓰기 */
                    putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
                    /* 마우스 그리기 */
                }
            }
        }
	}
}

#define EFLAGS_AC_BIT       0x00040000
#define CR0_CACHE_DISABLE   0x60000000

unsigned int memtest(unsigned int start, unsigned int end)
{
    char flg486 = 0;
    unsigned int eflg, cr0, i;

    /* 386인가, 486이후인가의 확인 */
    eflg = io_load_eflags();
    eflg |= EFLAGS_AC_BIT; /* AC-bit = 1 */
    io_store_eflags(eflg);
    eflg = io_load_eflags();
    if ((eflg & EFLAGS_AC_BIT) != 0) { /* 386에서는 AC=1으로 해도 자동으로 0에 돌아와 버린다 */
        flg486 = 1;
    }
    eflg &= ~EFLAGS_AC_BIT; /* AC-bit = 0 */
    io_store_eflags(eflg);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 |= CR0_CACHE_DISABLE; /* 캐쉬 금지 */
        store_cr0(cr0);
    }

    i = memtest_sub(start, end);

    if (flg486 != 0) {
        cr0 = load_cr0();
        cr0 &= ~CR0_CACHE_DISABLE; /* 캐쉬 허가 */
        store_cr0(cr0);
    }

    return i;
}

void memman_init(struct MEMMAN *man)
{
    man->frees = 0;         /* 빈 정보 개수 */
    man->maxfrees = 0;      /* 상황 관찰용: frees의 최대 값 */
    man->lostsize = 0;      /* 해제에 실패한 합계 사이즈 */
    man->losts = 0;         /* 해제에 실패한 횟수 */
    return;
}

unsigned int memman_total(struct MEMMAN *man)
/* 빈 사이즈의 합계를 보고 */
{
    unsigned int i, t = 0;
    for (i = 0; i < man->frees; i++) {
        t += man->free[i].size;
    }
    return t;
}

unsigned int memman_alloc(struct MEMMAN *man, unsigned int size)
/* 확보 */
{
    unsigned int i, a;
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].size >= size) {
            /* 충분한 넓이의 빈 영역을 발견 */
            a = man->free[i].addr;
            man->free[i].addr += size;
            man->free[i].size -= size;
            if (man->free[i].size == 0) {
                /* free[i]가 없어졌으므로 앞으로 채운다. */
                man->frees--;
                for (; i < man->frees; i++) {
                    man->free[i] = man->free[i + 1];    /* 구조체의 대입 */
                }
            }
            return a;
        }
    }
    return 0;       /* 빈 영역이 없다. */
}

int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size)
/* 해제 */
{
    int i, j;
    /* 정리하기 쉽게 하려면, free[]를 addr 순서로 나열하는 편이 좋다. */
    /* 그러니까 우선 어디에 넣을 것인지 결정한다. */
    for (i = 0; i < man->frees; i++) {
        if (man->free[i].addr > addr) {
            break;
        }
    }
    /* free[i - 1].addr < addr < free[i].addr */
    if (i > 0) {
        /* 앞이 비어있지 않다. */
        if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
            /* 앞의 빈 영역을 정리한다. */
            man->free[i - 1].size += size;
            if (i < man->frees) {
                /* 뒤에도 비어 있지 않다. */
                if (addr + size == man->free[i].addr) {
                    /* 뒤의 빈 영역도 정리한다. */
                    man->free[i - 1].size += man->free[i].size;
                    /* man->free[i]의 삭제 */
                    /* free[i]가 없어졌으므로 앞으로 채운다. */
                    man->frees--;
                    for (; i < man->frees; i++) {
                        man->free[i] = man->free[i + 1];    /* 구조체의 대입 */
                    }
                }
            }
            return 0;   /* 성공 종료 */
        }
    }
    /* 앞의 빈 영역이 정리되지 않았다. */
    if (i < man->frees) {
        /* 뒤에 빈 영역이 정리되지 않았다. */
        if (addr + size == man->free[i].addr) {
            /* 뒤의 빈 영역을 정리한다. */
            man->free[i].addr = addr;
            man->free[i].size += size;
            return 0;   /* 성공 종료 */
        }
    }
    /* 앞에도 뒤에도 빈 영역이 없다. */
    if (man->frees < MEMMAN_FREES) {
        /* free[i]보다 뒤의 빈 영역을 뒤로 옮겨 놓고, 간격을 만든다. */
        for (j = man->frees; j > i; j--) {
            man->free[j] = man->free[j - 1];
        }
        man->frees++;
        if (man->maxfrees < man->frees) {
            man->maxfrees = man->frees;     /* 최대 값을 갱신 */
        }
        man->free[i].addr = addr;
        man->free[i].size = size;
        return 0;   /* 성공 종료 */
    }
    man->losts++;
    man->lostsize += size;
    return -1;  /* 실패 종료 */
}
