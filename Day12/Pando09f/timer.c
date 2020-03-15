#include "bootpack.h"

#define PIT_CTRL    0x0043
#define PIT_CNT0    0x0040
#define TIMER_FLAGS_ALLOC   1   /* 확보한 상태 */
#define TIMER_FLAGS_USING   2   /* 타이머 작동 중 */

struct TIMERCTL timerctl;

void init_pit(void)
{
    int i;
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    timerctl.count = 0;
    timerctl.next = 0xffffffff; /* 처음에는 작동 중의 타이머가 없기 때문에 */
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timer[i].flags = 0; /* 미사용 */
    }
    return;
}

struct TIMER *timer_alloc(void)
{
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer[i].flags == 0) {
            timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timer[i];
        }
    }
    return 0;   /* 발견되지 않았다. */
}

void timer_free(struct TIMER *timer)
{
    timer->flags = 0; /* 미사용 */
    return;
}

void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data)
{
    timer->fifo = fifo;
    timer->data = data;
    return;
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    if (timerctl.next > timer->timeout) {
        /* 다음 시각을 갱신 */
        timerctl.next = timer->timeout;
    }
    return;
}

void inthandler20(int *esp)
{
    int i;
    io_out8(PIC0_OCW2, 0x60);   /* IRQ-00 접수 완료를 PIC에 통지 */
    timerctl.count++;
    if (timerctl.next > timerctl.count) {
        return; /* 다음 시각이 없기 때문에, 이제 끝 */
    }
    timerctl.next = 0xffffffff;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {     /* 타임아웃이 설정되어 있다. */
            if (timerctl.timer[i].timeout <= timerctl.count) {
                /* 타임아웃 */
                timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
                fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
            } else {
                /* 아직 타임아웃이 아니다. */
                if (timerctl.next > timerctl.timer[i].timeout) {
                    timerctl.next = timerctl.timer[i].timeout;
                }
            }
        }
    }
    return;
}
