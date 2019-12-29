; hello-os
; TAB=4

		ORG		0x7c00			; 메모리 내 어디에 로딩되는가

; 이하는 표준적인 FAT12 포맷 플로피디스크를 위한 서술

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"	    ; boot sector의 이름을 자유롭게 써도 좋음
		DW		512			    ; 1섹터의 크기 (바이트 단위. 512로 해야 함)
		DB		1			    ; 클러스터의 크기 (1섹터로 해야 함)
		DW		1			    ; 예약된 섹터의 수
		DB		2			    ; 디스크의 FAT 테이블의 수
		DW		224			    ; 루트 디렉토리 엔트리의 수 (보통은 224엔트리)
		DW		2880		    ; 디스크의 총 섹터 수 (2880섹터로 해야 함)
		DB		0xf0		    ; 미디어 타입 (0xf0으로 해야 함)
		DW		9			    ; 하나의 FAT 테이블의 섹터 수 (9섹터로 해야 함)
		DW		18			    ; 1트랙에 몇 섹터가 있는가(18로 해야 함)
		DW		2			    ; 헤드의 수 (2로 해야함)
		DD		0			    ; 파티션을 사용하지 않으므로 이곳은 반드시 0
		DD		2880			; 이 드라이브의 크기를 한 번 더 씀
		DB		0,0,0x29		; 잘 모르겠지만 이 값을 넣어 두면 좋다고 함
		DD		0xffffffff		; 아마 볼륨 시리얼 번호
		DB		"PANDO-OS   "	; 디스크의 이름
		DB		"FAT12   "		; 포맷의 이름 (8바이트)
		RESB	18				; 어쨌든 18바이트 남겨둠

; 프로그램 본체

entry:
		MOV		AX, 0			; 레지스터 초기화
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX
		MOV		ES,AX

		MOV		SI,msg
putloop:
		MOV		AL,[SI]
		ADD		SI, 1			; SI에 1을 더한다.
		CMP		AL,0
		JE		fin
		MOV		AH, 0x0e		; 한 문자 표시 기능
		MOV		BX, 15			; 컬러 코드
		INT		0x10			; 비디오 BIOS 호출
		JMP		putloop
fin:
		HLT					    ; CPU를 정지시킴
		JMP		fin			    ; 무한 루프

msg:
		DB		0x0a, 0x0a		; 줄 바꿈 문자 2개
		DB		"Welcome to PandOS"
		DB		0x0a			; 줄 바꿈
		DB		0

		RESB	0x7dfe-0x7c00-($-$$)	; 0x7dfe까지를 0x00으로 채우는 명령

		DB		0x55, 0xaa

