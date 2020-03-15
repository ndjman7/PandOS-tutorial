; iolib
; TAB=4

[bits 32]					; 32비트 모드용의 기계어를 만든다

        GLOBAL	io_hlt, io_cli, io_sti, io_stihlt
        GLOBAL  io_in8, io_in16, io_in32
        GLOBAL  io_out8, io_out16, io_out32
        GLOBAL  io_load_eflags, io_store_eflags
        GLOBAL  load_gdtr, load_idtr
        GLOBAL  load_cr0, store_cr0
        GLOBAL  asm_inthandler21, asm_inthandler2c
        GLOBAL  asm_inthandler20
        GLOBAL  memtest_sub
        EXTERN  inthandler21, inthandler2c
        EXTERN  inthandler20

_start:
    [extern HariMain] ; Define calling point. Must have same name as kernel.c 'main' function
    call HariMain ; Calls the C function. The linker will know where it is placed in memory

io_hlt:	; void io_hlt(void);
	    HLT
	    RET

io_cli:    ; void io_cli(void);
        CLI
        RET

io_sti:    ; void io_sti(void);
        STI
        RET

io_stihlt: ; void io_stihlt(void);
        STI
        HLT
        RET

io_in8:    ; int io_in8(int port);
        MOV     EDX, [ESP+4]    ; port
        MOV     EAX, 0
        IN      AL, DX
        RET

io_in16:   ; int io_in16(int port);
        MOV     EDX, [ESP+4]    ; port
        MOV     EAX, 0
        IN      AX, DX
        RET

io_in32:   ; int io_in32(int port);
        MOV     EDX, [ESP+4]    ; port
        IN      EAX, DX
        RET

io_out8:    ; void io_out8(int port, int data);
        MOV     EDX, [ESP+4]    ; port
        MOV     AL, [ESP+8]     ; data
        OUT     DX, AL
        RET

io_out16:   ; void io_out16(int port, int data);
        MOV     EDX, [ESP+4]    ; port
        MOV     EAX, [ESP+8]    ; data
        OUT     DX, AX
        RET

io_out32:   ; void io_out32(int port, int data);
        MOV     EDX, [ESP+4]    ; port
        MOV     EAX, [ESP+8]    ; data
        OUT     DX, EAX
        RET

io_load_eflags:    ; int io_load_eflags(void);
        PUSHFD      ; PUSH EFLAGS라는 의미
        POP     EAX
        RET

io_store_eflags:   ; void io_store_eflags(int eflags);
        MOV     EAX, [ESP+4]
        PUSH    EAX
        POPFD       ; POP EFLAGS라는 의미
        RET

load_gdtr:  ; void load_gdtr(int limit, int addr);
        MOV     AX, [ESP+4]     ; limit
        MOV     [ESP+6], AX
        LGDT    [ESP+6]
        RET

load_idtr:  ; void load_idtr(int limit, int addr);
        MOV     AX, [ESP+4]     ; limit
        MOV     [ESP+6], AX
        LIDT    [ESP+6]
        RET

asm_inthandler21:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV     EAX, ESP
        PUSH    EAX
        MOV     AX, SS
        MOV     DS, AX
        MOV     ES, AX
        CALL    inthandler21
        POP     EAX
        POPAD
        POP     DS
        POP     ES
        IRETD

asm_inthandler2c:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV     EAX, ESP
        PUSH    EAX
        MOV     AX, SS
        MOV     DS, AX
        MOV     ES, AX
        CALL    inthandler2c
        POP     EAX
        POPAD
        POP     DS
        POP     ES
        IRETD

load_cr0:   ; int load_cr0(void);
        MOV     EAX, CR0
        RET

store_cr0:  ; void store_cr0(int cr0);
        MOV     EAX, [ESP+4]
        MOV     CR0, EAX
        RET

memtest_sub:    ; unsigned int memtest_sub(unsigned int start, unsigned int end)
        PUSH    EDI     ; (EBX, ESI, EDI도 사용하고 싶기 때문에)
        PUSH    ESI
        PUSH    EBX
        MOV     ESI, 0xaa55aa55
        MOV     EDI, 0x55aa55aa
        MOV     EAX, [ESP+12+4]
mts_loop:
        MOV     EBX, EAX
        ADD     EBX, 0xffc
        MOV     EDX, [EBX]
        MOV     [EBX], ESI
        XOR     DWORD [EBX], 0xffffffff
        CMP     EDI, [EBX]
		JNE		mts_fin
		XOR		DWORD [EBX], 0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p ! = pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX], EDX				; *p = old;
		ADD		EAX, 0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX], EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

asm_inthandler20:
        PUSH    ES
        PUSH    DS
        PUSHAD
        MOV     EAX, ESP
        PUSH    EAX
        MOV     AX, SS
        MOV     DS, AX
        MOV     ES, AX
        CALL    inthandler20
        POP     EAX
        POPAD
        POP     DS
        POP     ES
        IRETD
