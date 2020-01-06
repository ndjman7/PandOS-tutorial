; pando-os
; TAB-4

; BOOT_INFO 관계
CYLS    EQU     0x0ff0          ; 부트섹터가 설정한다.
LEDS    EQU     0x0ff1
VMODE   EQU     0x0ff2          ; 색 수에 관한 정보. 몇 비트 컬러인가.
SCRNX   EQU     0x0ff4          ; 해상도 X (screen x)
SCRNY   EQU     0x0ff6          ; 해상도 Y (screen y)
VRAM    EQU     0x0ff8          ; 그래픽 버퍼의 개시 번지

        ORG     0xc200          ; 이 프러그램이 어디에 로딩되는가

        MOV     AL, 0x13        ; VGA 그래픽스. 320x200x8bit 컬러
        MOV     AH, 0x00
        INT     0x10
        MOV     BYTE [VMODE], 8 ; 화면 모드를 메모한다.
        MOV     WORD [SCRNX], 320
        MOV     WORD [SCRNY], 200
        MOV     DWORD [VRAM], 0x000a0000

; 키보드의 LED 상태를 BIOS가 알려준다.

        MOV     AH, 0x02        ; keyboard BIOS
        INT     0x16
        MOV     [LEDS], AL

fin:
        HLT
        JMP     fin
