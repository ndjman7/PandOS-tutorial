#include "bootpack.h"
#include <stdio.h>

void init_pic(void)
/* PIC의 초기화 */
{
    io_out8(PIC0_IMR,   0xff    );  /* 모든 인터럽트를 막는다. */
    io_out8(PIC1_IMR,   0xff    );  /* 모든 인터럽트를 막는다. */

    io_out8(PIC0_ICW1,  0x11    );  /* 엣지 트리거 모드 */
    io_out8(PIC0_ICW2,  0x20    );  /* IRQ0-7은 INT20-27에서 받는다. */
    io_out8(PIC0_ICW3,  1 << 2  );  /* PIC1은 IRQ2에 접속 */
    io_out8(PIC0_ICW4,  0x01    );  /* 논버퍼 모드 */

    io_out8(PIC1_ICW1,  0x11    );  /* 엣지 트리거 모드 */
    io_out8(PIC1_ICW2,  0x28    );  /* IRQ8-15는 INT28-2f에서 받는다. */
    io_out8(PIC1_ICW3,  2       );  /* PIC1은 IRQ2에 접속 */
    io_out8(PIC1_ICW4,  0x01    );  /* 논버퍼 모드 */

    io_out8(PIC0_IMR,   0xfb    );  /* 11111011 PIC1 이외는 모두 금지 */
    io_out8(PIC1_IMR,   0xff    );  /* 11111111 모든 인터럽트를 받지 않는다. */

    return;
}

#define PORT_KEYDAT     0x0060

struct FIFO8 keyfifo;

void inthandler21(int *esp)
/* PS/2 키보드로부터의 인터럽트 */
{
    unsigned char data;
    io_out8(PIC0_OCW2, 0x61);   /* IRQ-01 접수 완료를 PIC에 통지 */
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&keyfifo, data);
    return;
}

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
/* PS/2 마우스로부터의 인터럽트 */
{
    unsigned char data;
    io_out8(PIC1_OCW2, 0x64); /* IRQ-12 접수 완료를 PIC1에 통지 */
    io_out8(PIC0_OCW2, 0x62); /* IRQ-12 접수 완료를 PIC0에 통지 */
    data = io_in8(PORT_KEYDAT);
    fifo8_put(&mousefifo, data);
    return;
}
