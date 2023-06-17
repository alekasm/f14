; file: global.inc

VGA_ONLY 	equ	1
EGA_ONLY	equ	0
TARGETING	equ	0

	IF	VGA_ONLY

COMPRESSION	EQU	0

	ENDIF

	IF	EGA_ONLY

COMPRESSION	EQU	1

	ENDIF

; END OF FILE

