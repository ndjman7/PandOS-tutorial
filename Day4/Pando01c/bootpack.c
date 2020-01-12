void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{
	int i; /* 변수 선언.i라고 하는 변수는 32 비트의 정수형 */
    char *p; /* p라는 변수는 BYTE [...]용의 번지 */

	for (i = 0xa0000; i <= 0xaffff; i++) {

        p = (char *) i;
        *p = i & 0x0f;

        /* 이것으로 write_mem8(i, i & 0x0f); 대신 사용할 수 있음 */
	}

	for (;;) {
		io_hlt();
	}
}
