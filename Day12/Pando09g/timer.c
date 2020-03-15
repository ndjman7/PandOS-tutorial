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
    timerctl.using = 0;
    for (i = 0; i < MAX_TIMER; i++) {
        timerctl.timers0[i].flags = 0; /* 미사용 */
    }
    return;
}

struct TIMER *timer_alloc(void)
{
    int i;
    for (i = 0; i < MAX_TIMER; i++) {
        if (timerctl.timers0[i].flags == 0) {
            timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
            return &timerctl.timers0[i];
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
    int e, i, j;
    timer->timeout = timeout + timerctl.count;
    timer->flags = TIMER_FLAGS_USING;
    e = io_load_eflags();
    io_cli();
    /* 어디에 들어가면 되는지를 탐색한다. */
    for (i = 0; i < timerctl.using; i++) {
        if (timerctl.timers[i]->timeout >= timer->timeout) {
            break;
        }
    }
    /* 뒤로 옮겨 놓는다. */
    for (j = timerctl.using; j > i; j--) {
        timerctl.timers[j] = timerctl.timers[j - 1];
    }
    timerctl.using++;
    /* 빈 곳에 넣는다. */
    timerctl.timers[i] = timer;
    timerctl.next = timerctl.timers[0]->timeout;
    io_store_eflags(e);
    return;
}

void inthandler20(int *esp)
{
    int i, j;
    io_out8(PIC0_OCW2, 0x60);   /* IRQ-00 접수 완료를 PIC에 통지 */
    timerctl.count++;
    if (timerctl.next > timerctl.count) {
        return; /* 다음 시각이 없기 때문에, 이제 끝 */
    }
    for (i = 0; i < timerctl.using; i++) {
        /* timers의 타이머는 모두 동작 중이므로, flags를 확인하지 않는다. */
        if (timerctl.timers[i]->timeout > timerctl.count) {
            break;
        }
        /* timeout */
        timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
        fifo8_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
    }
    /* 정확히 i개의 타이머가 타임아웃되었다. 나머지를 다른 곳으로 옮겨 놓는다. */
    timerctl.using -= i;
    for (j = 0; j < timerctl.using; j++) {
        timerctl.timers[j] = timerctl.timers[i + j];
    }
    if (timerctl.using > 0) {
        timerctl.next = timerctl.timers[0]->timeout;
    } else {
        timerctl.next = 0xffffffff;
    }
    return;
}
