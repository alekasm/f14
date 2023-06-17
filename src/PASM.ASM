	DOSSEG
	.MODEL MEDIUM,C

	PUBLIC	NGXLO32,NGYLO32,NGXHI32,NGYHI32
	PUBLIC	NGXLO,NGYLO,NGXHI,NGYHI,NGXMID,NGYMID
	PUBLIC	Nviewmat,NTRANSGREYPTRS,GRTBLSEG


;	extrn	FlagRotate3D:PROC
	extrn	CLIPZ:DWORD
	extrn	MULT1X3:PROC, MULT3X3:PROC, COPY3X3:PROC, TRANS3X3:PROC
        extrn   OBJ:WORD,BSPBASE:DWORD,FBPTR:WORD,DRAWPTR:WORD



SMUL	MACRO	val
	imul	WORD PTR val
	shl	ax,1
	rcl	dx,1
	ENDM
SMULREG	MACRO	reg
	imul	reg
	shl	ax,1
	rcl	dx,1
	ENDM

ABSAX	MACRO
	cwd
	xor	ax,dx
	sub	ax,dx
	ENDM

ABSAXDX	MACRO
	LOCAL	abs_ok
	or	dx,dx
	jns	abs_ok
	not	ax
	not	dx
	add	ax,1
	adc	dx,0
abs_ok:
	ENDM
SAR4	MACRO	reg
	sar	reg,1
	sar	reg,1
	sar	reg,1
	sar	reg,1
	ENDM
SHL4	MACRO	reg
	shl	reg,1
	shl	reg,1
	shl	reg,1
	shl	reg,1
	ENDM
WALKDWN	MACRO			;ax has hi! word, dx has lo word
	mov	dl,dh
	mov	dh,al
	mov	al,ah
	cbw
	xchg	ax,dx
	ENDM
WALKUP	MACRO			;ax has hi! word, dx has garbage
	cwd
	mov	dl,ah
	mov	ah,al
	sub	al,al
	ENDM

IV	equ	0
C1	equ	IV
JV	equ	2
C2	equ	JV
KV	equ	4
C3	equ	KV
R1	equ	0
R2	equ	6
R3	equ	12
C1LO	equ	0
C1HI	equ	2
C2LO	equ	4
C2HI	equ	6
C3LO	equ	8
C3HI	equ	10


.DATA
NGXLO	label	word
NGXLO32	dd	0
NGYLO	label	word
NGYLO32	dd	0
NGXHI	label	word
NGXHI32	dd	319
NGYHI	label	word
NGYHI32	dd	199

NGXMID	dw	160
NGYMID	dw	100


FILLTBL	label	word
FILLYLO	dd	07fffh	;ylo
FILLYHI	dd	-1	;yhi


Nviewmat		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh ;view and inverse matrixes
		dw	7fffh,0,0, 0,7fffh,0, 0,0,7fffh
zoomfactor	dw	7fffh			;amount to scale z val by
NOASPECT	dw	0

CNT	dw	0
DELZ	dd	0
NPX	dd	0
NPXT	dd	0
NPYT	dd	0
NPXADD	dd	0
NPXTADD	dd	0
NPYTADD	dd	0
TOPTOBOT        dw      0

VECT    dw      0,0,0
LVECT   dd      0,0,0

OFlags  dw      0
MWID    dw      0




TransTable	db	0,1,8,8,0,5,0,8,0,8,8,8,4,0,6,7
	db	16,18,26,18,31,31,31,31,31,31,31,31,31,28,24,16
	db	32,33,33,34,34,34,35,35,36,36,37,37,38,38,39,39
	db	48,49,49,50,50,50,51,51,52,53,53,53,54,55,55,55
	db	64,65,65,65,65,65,65,65,66,66,66,67,67,68,68,69
	db	80,80,81,81,82,82,83,83,84,84,85,85,85,85,85,85
	db	96,97,97,98,98,98,99,99,100,100,101,101,101,102,102,103
	db	112,113,113,113,114,114,115,115,116,116,117,117,117,118,118,118
	db	128,129,129,130,130,131,131,132,132,133,133,134,135,135,136,136
	db	144,145,155,156,153,152,155,145,152,153,153,154,154,154,155,155
	db	160,161,161,161,162,162,163,163,164,164,165,165,165,166,166,166
	db	176,177,177,177,178,178,179,179,180,180,181,181,181,182,182,183
	db	192,193,193,194,194,194,195,195,196,196,197,197,197,198,198,198
	db	208,208,209,210,210,210,211,211,212,213,213,214,215,215,216,216
	db	224,224,225,226,226,227,227,228,228,229,229,230,230,231,231,232
	db	240,241,241,242,242,242,243,243,244,244,245,245,245,246,246,240

NTRANSGREYPTRS   dw      TransTable



_FLLTBL	SEGMENT	PARA PUBLIC 'DAT'

FILLMIN	label	dword
	REPT	210
	dd	03fffffffh
	dw	0,0
	ENDM
FILLMAX	label	dword
	REPT	210
	dd	-1
	dw	0,0
	ENDM

_FLLTBL	ENDS


_GRTBL SEGMENT	PARA PUBLIC 'DAT2'
GRTBLSEG        label   byte
	db	0,1,2,3,4,0,6,7,8,9,10,11,6,13,14,6
	db	16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	db	32,33,34,35,35,37,37,39,40,41,42,43,44,45,46,47
	db	48,49,50,51,52,53,54,55,56,57,57,59,59,61,62,62
	db	64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79
	db	80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95
	db	96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111
	db	112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127
	db	128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143
	db	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159
	db	160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175
	db	176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191
	db	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207
	db	208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223
	db	224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239
	db	240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255

	db	0,2,3,3,4,0,6,7,8,9,10,11,6,13,14,6
	db	16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
	db	32,33,34,35,35,35,35,37,39,41,41,42,43,44,45,47
	db	48,49,50,51,52,53,53,54,55,56,56,56,56,57,62,62
	db	64,65,66,67,68,69,70,71,72,73,74,75,76,77,77,79
	db	80,81,82,83,84,84,85,86,87,88,89,90,92,93,94,95
	db	96,97,98,99,100,101,102,103,103,105,105,107,107,108,109,110
	db	112,113,114,115,116,117,118,119,119,121,121,122,123,124,125,126
	db	128,129,130,131,132,133,134,135,136,137,138,138,139,141,141,142
	db	144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,158
	db	160,161,162,163,164,165,166,167,168,169,169,170,171,172,173,174
	db	176,177,178,179,180,181,182,183,184,185,186,186,188,188,189,190
	db	192,193,194,195,196,197,198,199,199,201,201,202,203,204,205,205
	db	208,209,210,211,212,213,214,215,216,217,217,218,219,220,221,222
	db	224,225,226,227,228,229,230,231,232,233,233,234,235,236,237,238
	db	240,241,242,243,244,245,246,247,247,248,249,250,251,252,253,255

	db	0,7,3,3,4,0,1,7,8,9,10,11,1,13,14,1
	db	16,17,18,20,20,21,22,23,24,25,26,27,28,29,30,18
	db	32,33,34,35,35,35,35,35,39,40,41,42,43,44,45,46
	db	48,49,49,50,51,52,53,54,54,55,55,55,55,55,62,62
	db	64,65,66,67,68,69,69,70,71,72,73,74,75,76,77,77
	db	80,81,82,82,83,84,85,86,87,88,89,89,92,93,91,95
	db	96,97,98,99,100,100,101,102,103,104,105,106,106,107,108,109
	db	112,113,114,115,115,116,117,118,119,120,121,121,122,123,124,124
	db	128,129,130,131,132,132,133,134,135,136,137,138,139,140,141,141
	db	144,145,146,147,148,149,150,151,152,153,154,154,150,156,157,158
	db	160,161,162,163,164,164,165,166,167,168,169,169,170,171,172,173
	db	176,177,178,179,180,181,181,182,183,184,185,185,187,188,188,189
	db	192,193,194,195,196,196,197,198,199,200,201,201,202,203,204,204
	db	208,209,210,211,212,212,213,214,215,216,217,218,218,219,221,221
	db	224,225,226,227,228,228,229,230,231,232,233,234,235,236,236,237
	db	240,241,242,243,244,245,245,246,246,247,247,248,249,250,251,255

	db	0,7,7,3,4,0,1,7,8,9,10,11,1,13,14,1
	db	16,17,18,20,20,30,22,23,24,25,26,27,28,29,30,18
	db	32,33,34,35,35,35,35,35,37,40,41,42,42,43,44,46
	db	48,49,49,50,51,51,52,53,54,54,54,54,54,55,62,62
	db	64,65,66,67,67,68,69,70,71,72,73,74,74,75,76,76
	db	80,81,81,82,83,84,85,85,86,87,88,89,92,93,91,95
	db	96,97,98,99,99,100,101,102,102,103,104,105,106,106,107,107
	db	112,113,114,114,115,116,117,118,118,119,120,121,122,122,123,123
	db	128,129,130,131,131,132,133,134,135,136,136,137,138,139,140,141
	db	144,145,146,154,148,149,150,151,152,153,153,154,150,150,156,157
	db	160,161,162,163,163,164,165,166,166,167,168,169,170,170,171,171
	db	176,177,178,179,179,180,181,182,183,183,184,185,186,187,187,188
	db	192,193,194,195,195,196,197,198,198,199,200,201,201,202,202,203
	db	208,209,210,211,211,212,213,214,215,216,216,217,218,219,220,220
	db	224,225,226,227,227,228,229,230,231,231,232,233,234,235,235,236
	db	240,241,242,243,243,244,245,246,246,246,246,247,247,248,249,255

	db	0,7,7,3,4,0,1,4,8,9,7,11,1,13,14,1
	db	16,17,17,20,20,30,22,23,24,25,26,27,28,28,17,18
	db	32,33,34,35,35,35,35,35,37,39,40,41,42,42,43,45
	db	48,49,49,50,50,51,51,52,53,53,53,53,53,54,61,61
	db	64,65,66,66,67,68,69,70,71,71,72,73,74,75,75,75
	db	80,81,81,82,83,84,84,85,86,87,87,88,92,93,91,95
	db	96,97,98,98,99,100,100,101,102,103,103,104,105,105,106,107
	db	112,113,113,114,115,116,117,117,118,119,119,120,121,121,122,122
	db	128,129,130,130,131,132,133,134,134,135,136,136,137,138,139,139
	db	144,145,146,154,148,149,150,151,152,153,153,154,150,150,150,157
	db	160,161,161,162,163,164,164,165,166,167,167,168,169,169,170,170
	db	176,177,178,178,179,180,180,181,182,183,183,184,185,186,186,187
	db	192,193,194,194,195,196,197,197,198,199,199,200,201,201,202,202
	db	208,209,209,210,211,212,213,213,214,215,216,216,217,217,218,219
	db	224,225,226,226,227,228,228,229,230,231,231,233,233,234,234,235
	db	240,241,242,242,243,244,244,245,245,246,246,246,247,247,247,255

	db	0,7,7,8,8,0,7,8,8,9,7,13,7,13,14,7
	db	16,17,17,20,24,18,22,23,24,26,26,27,24,17,17,18
	db	32,33,34,35,35,35,35,35,35,37,40,41,41,42,43,45
	db	48,49,49,49,50,50,51,52,52,52,52,53,53,53,61,61
	db	64,65,66,66,67,68,68,69,70,71,72,73,73,74,75,75
	db	80,81,81,82,83,83,84,84,85,86,87,87,92,92,90,95
	db	96,97,97,98,99,99,100,101,101,102,103,103,104,105,105,106
	db	112,113,113,114,115,115,116,117,117,118,119,119,120,120,121,122
	db	128,129,129,130,131,131,132,133,134,134,135,136,137,137,138,139
	db	144,145,152,153,148,149,150,151,152,153,153,153,150,150,150,150
	db	160,161,161,162,163,163,164,165,165,166,167,167,168,169,169,169
	db	176,177,178,178,179,179,180,181,182,182,183,183,184,185,185,186
	db	192,193,193,194,195,195,196,197,197,198,199,199,200,200,201,201
	db	208,209,209,210,211,211,212,213,213,214,215,216,216,217,217,218
	db	224,225,226,226,227,228,228,229,230,230,231,232,232,233,234,234
	db	240,241,241,242,243,244,244,245,245,245,245,246,246,246,246,255

	db	0,8,8,8,8,0,7,8,8,8,4,13,7,13,8,7
	db	16,17,17,24,24,24,22,23,24,26,24,24,17,17,17,17
	db	32,33,34,35,35,35,35,35,35,37,39,40,41,41,42,44
	db	48,49,49,49,49,50,50,51,51,52,52,52,52,52,61,61
	db	64,65,65,66,67,67,68,69,70,70,71,72,73,74,74,74
	db	80,80,81,82,82,83,84,84,85,85,86,87,92,92,89,95
	db	96,97,97,98,98,99,100,100,101,101,102,103,103,104,104,104
	db	112,113,113,114,114,115,116,116,117,117,118,118,119,120,120,120
	db	128,129,129,130,131,131,132,133,133,134,135,135,136,137,137,138
	db	144,145,152,153,148,149,150,151,152,153,153,153,150,150,150,150
	db	160,161,161,162,163,163,164,164,165,166,166,167,167,168,168,169
	db	176,177,177,178,178,179,180,180,181,182,182,182,183,184,184,185
	db	192,193,193,194,194,195,196,196,197,197,198,198,199,199,200,200
	db	208,209,209,210,211,211,212,213,213,214,214,215,215,216,217,217
	db	224,225,225,226,227,227,228,228,229,230,230,231,232,232,233,234
	db	240,241,241,242,243,243,244,244,244,245,245,245,245,245,246,255

	db	0,8,8,8,8,0,7,8,8,8,8,13,7,13,8,7
	db	16,17,17,24,24,24,22,23,24,24,24,24,17,17,17,17
	db	32,33,34,35,35,35,35,35,35,35,37,39,40,41,41,43
	db	48,49,49,49,49,49,50,50,51,51,51,51,51,51,55,55
	db	64,65,65,66,66,67,68,69,69,70,71,71,72,73,73,74
	db	80,80,81,82,82,83,83,84,84,85,85,86,92,92,88,95
	db	96,97,97,98,98,99,99,100,100,101,101,102,102,103,103,104
	db	112,113,113,114,114,115,115,116,116,117,117,118,118,119,119,119
	db	128,129,129,130,130,131,131,132,133,133,134,134,135,136,136,137
	db	144,145,152,153,148,149,153,151,152,153,153,153,153,150,150,150
	db	160,161,161,162,162,163,163,164,164,165,165,166,166,167,167,168
	db	176,177,177,178,178,179,179,180,180,181,182,182,183,183,183,184
	db	192,193,193,194,194,195,195,196,196,197,197,198,198,198,199,199
	db	208,209,209,210,210,211,211,212,213,213,214,214,215,215,216,216
	db	224,225,225,226,226,227,227,228,228,229,230,230,231,231,232,233
	db	240,241,241,242,242,243,243,244,244,244,244,244,245,245,245,255

	db	0,8,8,8,8,0,7,8,0,8,8,13,7,0,8,7
	db	16,16,17,24,24,17,16,16,17,23,24,17,17,17,17,17
	db	32,33,32,32,32,35,35,35,35,35,35,39,39,40,41,42
	db	48,49,49,49,49,49,49,50,50,50,50,50,50,50,54,54
	db	64,65,65,66,66,67,67,68,69,69,70,71,71,72,73,73
	db	80,80,81,81,82,82,83,83,84,84,85,85,83,92,87,80
	db	96,97,97,98,98,98,99,99,100,100,101,101,101,102,102,103
	db	112,113,113,113,114,114,115,115,116,116,117,117,117,118,118,118
	db	128,129,129,130,130,130,131,132,132,133,133,134,134,135,136,136
	db	144,144,152,153,148,144,145,145,152,153,153,153,153,150,150,150
	db	160,161,161,161,162,162,163,163,164,164,165,165,165,166,166,166
	db	176,177,177,178,178,178,179,179,180,180,181,181,182,182,182,183
	db	192,193,193,194,194,194,195,195,196,196,197,197,197,198,198,198
	db	208,209,209,209,210,210,211,211,212,213,213,213,214,214,215,215
	db	224,224,225,226,226,227,227,227,228,228,229,230,230,231,231,231
	db	240,241,241,241,242,242,243,243,243,244,244,244,244,244,244,240

	db	0,8,8,8,8,0,8,8,0,8,8,13,8,0,8,8
	db	16,16,17,24,24,17,16,16,16,23,17,17,17,17,17,17
	db	32,32,32,32,32,32,32,32,35,35,35,35,39,39,40,41
	db	48,48,49,49,49,49,49,49,49,49,49,50,50,50,53,53
	db	64,65,65,65,66,66,67,67,68,69,69,70,71,71,72,72
	db	80,80,81,81,82,82,83,83,83,84,84,85,83,92,86,80
	db	96,97,97,97,98,98,98,99,99,100,100,100,101,101,102,102
	db	112,113,113,113,114,114,115,115,115,116,116,116,117,117,117,117
	db	128,129,129,129,130,130,131,131,132,132,133,133,134,134,135,135
	db	144,144,152,153,144,144,145,145,144,144,144,145,145,150,150,150
	db	160,161,161,161,162,162,163,163,163,164,164,164,165,165,165,166
	db	176,177,177,177,178,178,178,179,179,180,180,180,181,181,182,182
	db	192,193,193,193,194,194,194,195,195,196,196,196,197,197,197,197
	db	208,209,209,209,210,210,211,211,211,212,213,213,213,214,214,214
	db	224,224,225,226,226,226,227,227,228,228,228,229,229,230,230,231
	db	240,241,241,241,242,242,242,243,243,243,243,243,244,244,244,240

	db	0,8,8,8,13,0,8,8,0,8,8,13,8,0,8,8
	db	16,16,17,23,24,17,16,16,16,16,16,17,17,17,17,17
	db	32,32,32,32,32,32,32,32,32,35,35,35,35,34,39,40
	db	48,48,48,49,49,49,49,49,49,49,49,49,49,49,51,51
	db	64,65,65,65,66,66,67,67,67,68,69,69,70,70,71,71
	db	80,80,81,81,81,82,82,82,83,83,84,84,82,85,85,80
	db	96,97,97,97,98,98,98,99,99,99,99,100,100,100,101,101
	db	112,113,113,113,113,114,114,114,115,115,115,116,116,116,117,117
	db	128,129,129,129,129,130,130,131,131,132,132,132,133,133,134,134
	db	144,144,152,153,144,144,145,145,144,144,144,144,145,145,145,150
	db	160,161,161,161,161,162,162,162,163,163,163,164,164,164,165,165
	db	176,177,177,177,178,178,178,178,179,179,179,180,180,180,181,181
	db	192,192,193,193,194,194,194,194,195,195,195,196,196,196,196,196
	db	208,209,209,209,209,210,210,211,211,211,212,212,212,213,213,213
	db	224,224,225,225,226,226,226,227,227,227,228,228,228,229,229,230
	db	240,240,241,241,242,242,242,242,242,243,243,243,243,243,243,240

	db	0,8,8,8,13,0,8,8,0,0,8,13,8,0,8,8
	db	16,16,17,23,24,17,16,16,16,16,16,16,16,17,17,17
	db	32,32,32,32,32,32,32,32,32,35,35,35,35,35,37,34
	db	48,48,48,48,49,49,49,49,49,49,49,49,49,49,50,50
	db	64,64,65,65,65,66,66,67,67,67,68,69,69,69,70,70
	db	80,80,81,81,81,81,82,82,82,83,83,83,82,84,84,80
	db	96,96,97,97,97,98,98,98,98,99,99,99,99,99,100,100
	db	112,112,113,113,113,113,114,114,114,115,115,115,115,115,116,116
	db	128,128,129,129,129,129,130,130,131,131,131,132,132,133,133,133
	db	144,144,144,153,144,144,145,145,144,144,144,144,145,145,145,145
	db	160,160,161,161,161,162,162,162,162,163,163,163,163,163,164,164
	db	176,176,177,177,177,178,178,178,178,179,179,179,179,179,180,180
	db	192,192,193,193,193,194,194,194,194,194,195,195,195,195,195,196
	db	208,208,209,209,209,210,210,210,211,211,211,211,212,212,212,213
	db	224,224,225,225,226,226,226,226,227,227,227,227,228,228,228,229
	db	240,240,241,241,241,242,242,242,242,242,242,242,242,243,243,240

	db	0,8,8,8,13,0,8,8,0,0,8,0,8,0,8,8
	db	16,16,16,23,17,17,16,16,16,16,16,16,16,16,16,17
	db	32,32,32,32,32,32,32,32,32,32,32,35,35,35,35,34
	db	48,48,48,48,48,48,49,49,49,49,49,49,49,49,49,49
	db	64,64,65,65,65,65,66,66,66,67,67,67,68,69,69,69
	db	80,80,80,81,81,81,81,82,82,82,82,83,82,83,83,80
	db	96,96,97,97,97,97,97,98,98,98,98,98,99,99,99,99
	db	112,112,113,113,113,113,113,114,114,114,114,114,115,115,115,115
	db	128,128,129,129,129,129,130,130,130,130,131,131,131,132,132,132
	db	144,144,144,144,144,144,145,145,144,144,144,144,145,145,145,145
	db	160,160,161,161,161,161,161,162,162,162,162,162,162,163,163,163
	db	176,176,177,177,177,177,177,178,178,178,178,178,179,179,179,179
	db	192,192,193,193,193,193,193,194,194,194,194,194,194,195,195,195
	db	208,208,209,209,209,209,209,210,210,210,211,211,211,211,211,212
	db	224,224,224,225,225,226,226,226,226,226,227,227,227,227,227,228
	db	240,240,241,241,241,241,242,242,242,242,242,242,242,242,242,240

	db	0,0,0,0,0,0,8,0,0,0,0,0,8,0,0,8
	db	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
	db	32,32,32,32,32,32,32,32,32,32,32,32,32,32,35,35
	db	48,48,48,48,48,48,48,48,48,48,48,48,48,49,49,49
	db	64,64,65,65,65,65,65,65,66,66,66,67,67,67,68,67
	db	80,80,80,81,81,81,81,81,81,82,82,82,81,83,83,80
	db	96,96,96,97,97,97,97,97,97,98,98,98,98,98,98,98
	db	112,112,112,113,113,113,113,113,113,113,114,114,114,114,114,114
	db	128,128,129,129,129,129,129,129,129,130,130,130,130,131,131,131
	db	144,144,144,144,144,144,144,144,144,144,144,144,144,144,145,145
	db	160,160,160,161,161,161,161,161,161,162,162,162,162,162,162,162
	db	176,176,177,177,177,177,177,177,177,178,178,178,178,178,178,178
	db	192,192,192,193,193,193,193,193,193,194,194,194,194,194,194,194
	db	208,208,208,209,209,209,209,209,209,210,210,210,210,210,211,211
	db	224,224,224,225,225,225,225,226,226,226,226,226,226,227,227,227
	db	240,240,240,241,241,241,241,241,241,241,241,242,242,242,242,240

	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	db	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
	db	32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32
	db	48,48,48,48,48,48,48,48,48,48,48,48,48,48,49,49
	db	64,64,64,65,65,65,65,65,65,65,65,66,66,66,66,67
	db	80,80,80,80,80,81,81,81,81,81,81,81,81,82,82,80
	db	96,96,96,96,97,97,97,97,97,97,97,97,97,97,97,97
	db	112,112,112,112,113,113,113,113,113,113,113,113,113,113,113,113
	db	128,128,128,129,129,129,129,129,129,129,129,129,130,130,130,130
	db	144,144,144,144,144,144,144,144,144,144,144,144,144,144,144,144
	db	160,160,160,160,161,161,161,161,161,161,161,161,161,161,161,161
	db	176,176,176,177,177,177,177,177,177,177,177,177,177,177,178,178
	db	192,192,192,192,193,193,193,193,193,193,193,193,193,193,193,193
	db	208,208,208,208,209,209,209,209,209,209,209,209,209,209,210,210
	db	224,224,224,224,224,225,225,225,225,225,226,226,226,226,226,226
	db	240,240,240,240,241,241,241,241,241,241,241,241,241,241,241,240

	db	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
	db	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
	db	32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32
	db	48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48
	db	64,64,64,64,64,64,65,65,65,65,65,65,65,65,65,65
	db	80,80,80,80,80,80,80,80,80,81,81,81,80,81,81,80
	db	96,96,96,96,96,96,96,96,97,97,97,97,97,97,97,97
	db	112,112,112,112,112,112,112,112,113,113,113,113,113,113,113,113
	db	128,128,128,128,128,128,129,129,129,129,129,129,129,129,129,129
	db	144,144,144,144,144,144,144,144,144,144,144,144,144,144,144,144
	db	160,160,160,160,160,160,160,160,161,161,161,161,161,161,161,161
	db	176,176,176,176,176,176,176,177,177,177,177,177,177,177,177,177
	db	192,192,192,192,192,192,192,192,193,193,193,193,193,193,193,193
	db	208,208,208,208,208,208,208,208,209,209,209,209,209,209,209,209
	db	224,224,224,224,224,224,224,224,224,225,225,225,225,225,225,225
	db	240,240,240,240,240,240,240,240,240,240,241,241,241,241,241,240




_GRTBL  ENDS

.CODE
.386
XBITS	equ	11

;----------------------------------------------------------------------------


;struct	PT3D{
;	long	XR;
;	long	YR;
;	long	ZR;
;	long	XP;
;	long	YP;
;	long	ZP;
;	int	XT;
;	int	YT;
;	int	ZT;
;	int	FLAGS;
;	};
;// bits   15  14  13  12  11  10  09  08  07  06  05  04  03  02  01  00
;// FLAGS=  x   x   x  PD PYH PYL PXH PXL   x  RD RZH RZL RYH RYL RXH RXL
RXLFLAG	EQU	1
RXHFLAG	EQU	2
RYLFLAG	EQU	4
RYHFLAG	EQU	8
RZLFLAG	EQU	16
RZHFLAG	EQU	32
RDFLAG	EQU	64
ROUTS	EQU	63

PXLFLAG	EQU	256
PXHFLAG	EQU	512
PYLFLAG	EQU	1024
PYHFLAG	EQU	2048
PDFLAG	EQU	4096
POUTS	EQU	0f00h

PXR	equ	0
PYR	equ	4
PZR	equ	8
PXP	equ	12
PYP	equ	16
PZP	equ	20
PXT	equ	24
PYT	equ	26
PZT	equ	28
PFLAGS	equ	30
FlagRotate3D	PROC	uses es,P1:DWORD


	les	bx,P1

	mov	edx,es:[bx].PZR	;ZVAL

	mov	cx,0

	mov	eax,es:[bx].PXR
	cmp	eax,edx		;if (P->XR > ZVAL)	P->FLAGS |= RXHFLAG;
	jle	@F
	or	cx,RXHFLAG
@@:
	neg	eax		;if ( -P->XR > ZVAL) P->FLAGS |= RXLFLAG;
	cmp	eax,edx
	jle	@F
	or	cx,RXLFLAG
@@:
	mov	eax,es:[bx].PYR	;if (P->YR > ZVAL)	P->FLAGS |= RYHFLAG;
	cmp	eax,edx
	jle	@F
	or	cx,RYHFLAG
@@:
	neg	eax		;if ( -P->YR > ZVAL) P->FLAGS |= RYLFLAG;
	cmp	eax,edx
	jle	@F
	or	cx,RYLFLAG
@@:
	mov	eax,es:[bx].PZR	;P->ZR /=256;
;	sar	eax,8
	mov	es:[bx].PZR,eax
	cmp	eax,CLIPZ	;if (P->ZR < CLIPZ)	P->FLAGS |= RZLFLAG;
	jge	@F
	or	cx,RZLFLAG
@@:
;// for now only clip zlo
;// ignore the rdflag for now
        or      cx,RDFLAG

	mov	es:[bx].PFLAGS,cx
	ret

FlagRotate3D	ENDP

Perspect3D	PROC	uses si es,P1:DWORD


	les	si,P1

	mov	cx,es:[si].PFLAGS
	and	cx,0ffh
	mov	ebx,es:[si].PZR
	cmp	ebx,CLIPZ
	jl	done

        sar     ebx,8
	mov	eax,es:[si].PXR
	cdq
	idiv	ebx
	movzx	edx,NGXMID
	add	eax,edx
	mov	es:[si].PXP,eax
	cmp	eax,NGXLO32
	jge	@F
	or	cx,PXLFLAG
@@:
	cmp	eax,NGXHI32
	jle	@F
	or	cx,PXHFLAG
@@:

	mov	eax,es:[si].PYR
	cdq
	idiv	ebx
	movzx	edx,NGYMID
	add	eax,edx
	mov	es:[si].PYP,eax
	cmp	eax,NGYLO32
	jge	@F
	or	cx,PYLFLAG
@@:
	cmp	eax,NGYHI32
	jle	@F
	or	cx,PYHFLAG
@@:
	or	cx,PDFLAG
done:
	mov	es:[si].PFLAGS,cx
	ret
Perspect3D	ENDP

if 1

ZClip	PROC	uses	si di es fs gs,PIN:DWORD,POUT:DWORD,PDST:DWORD,TYP:WORD


PTIN	equ	es:[bx]
PTOUT	equ	fs:[si]
PTDST	equ	gs:[di]



	les	bx,PIN
	lfs	si,POUT
	lgs	di,PDST

	mov	ecx,PTOUT.PZR
	mov	eax,PTIN.PZR
	sub	ecx,eax
	mov	DELZ,ecx
	mov	ecx,CLIPZ
	sub	ecx,eax		;ecx= DZ

	mov	eax,PTOUT.PXR
	sub	eax,PTIN.PXR
	cdq
	imul	ecx
	idiv	DELZ
	add	eax,PTIN.PXR
	mov	PTDST.PXR,eax

	mov	eax,PTOUT.PYR
	sub	eax,PTIN.PYR
	cdq
	imul	ecx
	idiv	DELZ
	add	eax,PTIN.PYR
	mov	PTDST.PYR,eax

	mov	eax,CLIPZ
;        shl     eax,8
	mov	PTDST.PZR,eax

	mov	ax,TYP
	and	ax,3
	je	L1
	cmp	ax,3
	je	L1
	cmp	ax,1
	je	L2

	mov	ax,PTOUT.PYT
	sub	ax,PTIN.PYT
	cwde
	cdq
	imul	ecx
	idiv	DELZ
	add	ax,PTIN.PYT
	mov	PTDST.PYT,ax
L2:
	mov	ax,PTOUT.PXT
	sub	ax,PTIN.PXT
	cwde
	cdq
	imul	ecx
	idiv	DELZ
	add	ax,PTIN.PXT
	mov	PTDST.PXT,ax
L1:
	push	gs
	push	di
	call	FlagRotate3D
	add	sp,4

	ret
ZClip	ENDP

        endif
NPFLine	PROC	uses di si es fs gs,P1:DWORD,P2:DWORD,DIMS:WORD


P1X	equ	es:[di]
P1Y	equ	es:4[di]
P1Z	equ	es:8[di]
P1XT	equ	es:12[di]
P1YT	equ	es:14[di]
P2X	equ	fs:[si]
P2Y	equ	fs:4[si]
P2Z	equ	fs:8[si]
P2XT	equ	fs:12[si]
P2YT	equ	fs:14[si]

        mov     ax,SEG _FLLTBL
        mov     gs,ax

	push	bp

        mov     TOPTOBOT,1

	les	di,P1
	lfs	si,P2
	mov	eax,P2Y
	mov	ecx,P1Y
	cmp	eax,ecx		;bottom >= top
	jge	@F
;	xchg	di,si		;always go from top to bottom
	les	di,P2
	lfs	si,P1
	xchg	ecx,eax
        mov     TOPTOBOT,0
@@:
	cmp	eax,NGYLO32		;y2< top ?
	jl	DONE
	cmp	ecx,NGYHI32		;y1 >  bottom?
	jg	DONE

	sub	eax,ecx		;p2y-py1


	inc	eax		;+1 = delta y
;	mov	DELY,eax	;32 bit delta y
	mov	ebx,eax		;save in ebx
	mov	cx,DIMS
	and	cx,3
	je	L1		;1 dim fill
	cmp	cx,1
	je	L2


	mov	ax,P2YT
	mov	dx,P1YT
;	and	ax,0ffh		;maybe put this back
;	and	dx,0ffh		;maybe put this back
	sub	ax,dx
	shl	eax,16		;XBITS	;shift up for precision
	shl	edx,16
;	add	edx,08000h      ;put this back
	mov	NPYT,edx
	cdq
	idiv	ebx		;delyt /dely= add val
	mov	NPYTADD,eax
L2:
	mov	ax,P2XT
	mov	dx,P1XT
;	and	ax,0ffh		;maybe put this back
;	and	dx,0ffh		;maybe put this back
	sub	ax,dx
	shl	eax,16		;XBITS	;shift up for precision
	shl	edx,16
;	add	edx,08000h      ;put this back
	mov	NPXT,edx
	cdq
	idiv	ebx		;delxt /dely= add val
	mov	NPXTADD,eax
L1:
	mov	eax,P2X
	mov	edx,P1X
	sub	eax,edx
	shl	eax,XBITS	;shift up for precision
	shl	edx,XBITS
	mov	NPX,edx
	mov	edx,1           ;put back
	shl	edx,XBITS-1     ; why -1 ???????
	add	NPX,edx
	cdq
	idiv	ebx		;delx /dely= add val
	mov	NPXADD,eax

	mov	eax,P1Y		;get lo fill y
	mov	ecx,eax		;cx = ylo
	sub	eax,NGYLO32
	jge	@F

	neg	eax
	mov	ebx,eax 		;dy

	mov	eax,ebx		;cx= dy*NPYTADD
	imul	eax,NPYTADD
	add	NPYT,eax

	mov	eax,ebx		;cx= dy*NPXTADD
	imul	eax,NPXTADD
	add	NPXT,eax

	mov	eax,ebx		;cx= dy*NPXADD
	imul	eax,NPXADD
	add	NPX,eax

	mov	ecx,NGYLO32		;new ylo
@@:
	mov	eax,P2Y
	cmp	eax,NGYHI32
	jle	@F
	mov	eax,NGYHI32		;new yhi
@@:
	cmp	ecx,FILLYLO
	jge	@F
	mov	FILLYLO,ecx
@@:
        cmp	eax,FILLYHI
	jle	@F
	mov	FILLYHI,eax
@@:
        mov     edx,eax
        sub     edx,ecx
        inc     edx

        cmp     dx,2
        jl      SPLCASE


	lea	ebx,gs:FILLMIN
        test    TOPTOBOT,1
	je	SS100

	lea	ebx,gs:FILLMAX	;get max fill tbl

                
        mov     esi,NPXADD      ;max line pad if xdir is positive
        or      esi,esi
        js      SS101
        add     NPX,esi

        mov     esi,NPXTADD
        add     NPXT,esi
        mov     esi,NPYTADD
        add     NPYT,esi

        jmp     SS101           ;done

SS100:
        mov     esi,NPXADD      ;min line pad if xdir is negative
        or      esi,esi
        jns     SS101
        add     NPX,esi

        mov     esi,NPXTADD
        add     NPXT,esi
        mov     esi,NPYTADD
        add     NPYT,esi

SS101:



	mov	esi,ecx
	lea	esi,gs:[ebx+8*esi]	;get filltbl line address

	mov	cx,DIMS
	and	cx,3
	je	L4
	cmp	cx,1
	je	L5


	mov	cx,dx
	mov	di,si
	mov	eax,NPYT
	mov	ebx,NPYTADD
        shr     cx,1
        jnc     @F
	mov	gs:4[di],eax	;overstore lo 16 into NPXT to put hi 16 into yt
	add	eax,ebx
	add	di,8
@@:
        jcxz    L5
@@:
	mov	gs:4[di],eax	;overstore lo 16 into NPXT to put hi 16 into yt
	add	eax,ebx
	mov	gs:12[di],eax	;overstore lo 16 into NPXT to put hi 16 into yt
	add	eax,ebx
	add	di,16
	dec	cx		;put more iterations to amortizes the loop !!
	jne	@B
L5:
	mov	cx,dx
	mov	di,si
	mov	eax,NPXT
	mov	ebx,NPXTADD

        shr     cx,1
        jnc     @F
	mov	gs:2[di],eax	;overstore lo 16 into NPX+2 to put hi 16 into xt
	add	eax,ebx
	add	di,8
@@:
        jcxz    L4
@@:
	mov	gs:2[di],eax	;overstore lo 16 into NPX+2 to put hi 16 into xt
	add	eax,ebx
	mov	gs:10[di],eax	;overstore lo 16 into NPX+2 to put hi 16 into xt
	add	eax,ebx
	add	di,16
	dec	cx		;put more iterations to amortizes the loop !!
	jne	@B
L4:
	mov	cx,dx
	mov	di,si
	mov	eax,NPX
	mov	ebx,NPXADD
        
        shr     cx,1
        jnc     @F
	mov	gs:[di],eax	;store 32bits-let scan line fill sort it out
	add	eax,ebx
	add	di,8
@@:
        jcxz    DONE
@@:
	mov	gs:[di],eax	;store 32bits-let scan line fill sort it out
	add	eax,ebx
	mov	gs:8[di],eax	;store 32bits-let scan line fill sort it out
	add	eax,ebx
	add	di,16
	dec	cx		;put more iterations to amortizes the loop !!
	jne	@B
DONE:
	pop	bp
	ret
SPLCASE:

        mov     eax,NPXADD
        or      eax,eax
        js      SPLNEG


        mov     di,cx
        shl     di,3

        mov     CNT,dx
        mov     eax,NPX
        mov     edx,NPXADD
        mov     ebx,NPXT
        mov     esi,NPXTADD
        mov     ecx,NPYT
        mov     ebp,NPYTADD

SPLYT1:
        cmp     eax,gs:FILLMIN[di]
        jg      @F
	mov	gs:FILLMIN+4[di],ecx	;overstore lo 16 into NPXT to put hi 16 into yt
	mov	gs:FILLMIN+2[di],ebx	;overstore lo 16 into NPX+2 to put hi 16 into xt
	mov	gs:FILLMIN[di],eax	;store 32bits-let scan line fill sort it out
@@:
        add     ecx,ebp
        add     ebx,esi
        add     eax,edx
        cmp     eax,gs:FILLMAX[di]
        jl      @F
	mov	gs:FILLMAX+4[di],ecx	;overstore lo 16 into NPXT to put hi 16 into yt
	mov	gs:FILLMAX+2[di],ebx	;overstore lo 16 into NPX+2 to put hi 16 into xt
	mov	gs:FILLMAX[di],eax	;store 32bits-let scan line fill sort it out
@@:
        add     di,8
        dec     CNT
        jne     SPLYT1
        pop     bp
        ret
SPLNEG:
        mov     di,cx
        shl     di,3

        mov     CNT,dx
        mov     eax,NPX
        mov     edx,NPXADD
        mov     ebx,NPXT
        mov     esi,NPXTADD
        mov     ecx,NPYT
        mov     ebp,NPYTADD

SPLYT2:
        cmp     eax,gs:FILLMAX[di]
        jl      @F
	mov	gs:FILLMAX+4[di],ecx	;overstore lo 16 into NPXT to put hi 16 into yt
	mov	gs:FILLMAX+2[di],ebx	;overstore lo 16 into NPX+2 to put hi 16 into xt
	mov	gs:FILLMAX[di],eax	;store 32bits-let scan line fill sort it out
@@:
        add     ecx,ebp
        add     ebx,esi
        add     eax,edx
        cmp     eax,gs:FILLMIN[di]
        jg      @F
	mov	gs:FILLMIN+4[di],ecx	;overstore lo 16 into NPXT to put hi 16 into yt
	mov	gs:FILLMIN+2[di],ebx	;overstore lo 16 into NPX+2 to put hi 16 into xt
	mov	gs:FILLMIN[di],eax	;store 32bits-let scan line fill sort it out
@@:
        add     di,8
        dec     CNT
        jne     SPLYT2
        pop     bp
        ret

NPFLine	ENDP


if 0
NPFFill	PROC	uses di si es fs gs,DEST:DWORD,DIMS:WORD,SRC:DWORD

        mov     ax,SEG _FLLTBL
        mov     gs,ax

	mov	esi,FILLYLO
	cmp	esi,FILLYHI
	jg	DONE
	cmp	si,07fffh
	je	DONE		;no lines to fill
	mov	bx,si
	mov	edx,FILLYHI
	sub	dx,bx
	js	DONE		;bad-ylo>yhi
	inc	dx		;dx= # lines 1 based
	shl	esi,6
	lea	esi,gs:[esi+4*esi]
	add	DEST,esi	;DEST= yline screen addr to start
	shl	bx,3		;bx= start line *8 for minmaxtbls
	mov	cx,DIMS
	and	cx,3
	cmp	cx,2
	je	L3
	cmp	cx,1		;Gouroud-1dim fill
	je	L2

L1:
	les	si,DEST
	mov	eax,SRC
	push	bp
	mov	bp,ax

        ; do special type fills here
        cmp     ah,250                 ;transparent xlat fill
        jne     L10



L10a:
	mov	eax,gs:FILLMIN[bx]
	sar	eax,XBITS
	mov	ecx,gs:FILLMAX[bx]
	sar	ecx,XBITS
	cmp	ecx,eax
	jge	@F
	xchg	eax,ecx
@@:
	cmp	eax,NGXHI32
	jg	L11a
	cmp	ecx,NGXLO32
	jl	L11a
	cmp	eax,NGXLO32
	jge	@F
	mov	eax,NGXLO32
@@:
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,ax		;cx= #pixels
	inc	cx		;ax= start pix
	mov	di,si
	add	di,ax

        push    bx
        mov     bx,NTRANSGREYPTRS

@@:
	mov	al,es:[di]
	xlat
	stosb
	loop	@B
        pop     bx
L11a:
	add	bx,8
	add	si,320
	dec	dx
	jne	L10a
	pop	bp
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret


L10:
	mov	eax,gs:FILLMIN[bx]
	sar	eax,XBITS
	mov	ecx,gs:FILLMAX[bx]
	sar	ecx,XBITS
	cmp	ecx,eax
	jge	@F

;	jmp	L11                     ;vert case fix

	xchg	eax,ecx
@@:
	cmp	eax,NGXHI32
	jg	L11
	cmp	ecx,NGXLO32
	jl	L11
	cmp	eax,NGXLO32
	jge	@F
	mov	eax,NGXLO32
@@:
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,ax		;cx= #pixels
	inc	cx		;ax= start pix
	mov	di,si
	add	di,ax
	mov	ax,bp		;color word
	test	bx,08h		;line <<3
	je	@F
	xchg	al,ah
@@:
	test	di,1
	je	@F
	mov	es:[di],ah
	inc	di
	dec	cx
	je	L11
@@:
        cmp     cx,8
        jb      @F
        push    ax
        shl     eax,16
        pop     ax
        push    cx
        shr     cx,2
        rep     stosd
        pop     cx
        and     cx,3
@@:
	shr	cx,1
	je	@F
	rep	stosw
@@:
	jnc	L11
	mov	es:[di],al
L11:
	add	bx,8
	add	si,320
	dec	dx
	jne	L10
	pop	bp
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret
L2:
;1 dim fill -gouroud fill
;DEST= start of top yline
;bx=8*y line # (for min,max tbls)
;dx= # lines to do 1 based


	mov	CNT,dx


L20:
	les	di,DEST
	mov	esi,gs:FILLMIN[bx]
	sar	esi,XBITS
	mov	ecx,gs:FILLMAX[bx]
	sar	ecx,XBITS
	cmp	ecx,esi
	jge	@F

;	jmp	L21                     ;vert case fix

	xchg	esi,ecx
@@:
	cmp	esi,NGXHI32
	jg	L21
	cmp	ecx,NGXLO32
	jl	L21

;	jmp	L21		;!!!!!!!!!!!!!!!!!!!!!!!!

	push	ecx
	sub	ecx,esi
	inc	ecx

	mov	ax,WORD PTR gs:FILLMAX+4[bx]
	sub	ax,WORD PTR gs:FILLMIN+4[bx]
	shl	eax,16
	cdq
	idiv	ecx			;eax = xtadd<<16
	mov	ecx,eax			;ecx= xtadd<<16
	mov	dx,WORD PTR gs:FILLMIN+4[bx]
	shl	edx,16			;edx= xtstart
	cmp	esi,NGXLO32		;need to clip to left?
	jge	@F
	mov	eax,NGXLO32
	xchg	eax,esi			;esi=NGXLO32, eax= old xval
	sub	eax,esi
	neg	eax
	imul	eax,ecx			;dx*xtadd
	add	edx,eax			;xt+= dx*xtadd
@@:
;edx= xtstart, ecx= xtadd, esi=xstart
	add	di,si
	mov	ax,si			;ax= xlo
	mov	esi,ecx			;esi= xtadd
	shl	edx,8
	shl	esi,8
	pop	ecx			;xhi
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,ax			;cx= end-start
	inc	cx
	and	ecx,0ffffh		;clear out top 16 bits
@@:
	test	di,1
	je	@F
	shld	eax,edx,8
	add	edx,esi
	mov	es:[di],al
	inc	di
	dec	cx
	je	L21
@@:
	ror	ecx,3			;save 0-3 bit in 31-29 bit for later
	jcxz	@F
L22:
	shld	eax,edx,8
        mov     es:[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:1[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:2[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:3[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:4[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:5[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:6[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:7[di],al
	add	edx,esi
        add	di,8
	dec	cx
	jne	L22
@@:
	shl	ecx,1
	jnc	@F
	shld	eax,edx,8
        mov     es:[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:1[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:2[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:3[di],al
	add	edx,esi
	add	di,4
@@:
	shl	ecx,1
	jnc	@F
	shld	eax,edx,8
        mov     es:[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:1[di],al
	add	edx,esi
	add	di,2
@@:
	shl	ecx,1
	jnc	L21
	shld	eax,edx,8
	mov	es:[di],al
;	add	edx,esi
L21:
	add	bx,8
	add	DEST,320
	dec	CNT
	jne	L20
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret
;2 dim fill
L3:
	lfs	si,SRC
	shr	si,4
	mov	ax,fs
	add	ax,si
	mov	fs,ax
L30:
	push	bx
	push	dx
	push	ebp
L31:
	les	di,DEST
	mov	esi,gs:FILLMIN[bx]
	sar	esi,XBITS
	mov	ecx,gs:FILLMAX[bx]
	sar	ecx,XBITS
	cmp	ecx,esi
	jge	@F

;	jmp	L33                     ;vert case fix

	xchg	esi,ecx
@@:
	cmp	esi,NGXHI32
	jg	L33
	cmp	ecx,NGXLO32
	jl	L33


;	jmp	L33			;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	push	DIMS

	push	ecx
	sub	ecx,esi
	inc	ecx
	mov	ax,WORD PTR gs:FILLMAX+6[bx]
	sub	ax,WORD PTR gs:FILLMIN+6[bx]
	shl	eax,16
	cdq
	idiv	ecx			;eax = ytadd<<16
	mov	ebp,eax			;ebp = YTADD
	mov	ax,WORD PTR gs:FILLMAX+4[bx]
	sub	ax,WORD PTR gs:FILLMIN+4[bx]
	shl	eax,16
	cdq
	idiv	ecx			;eax = xtadd<<16
	mov	edx,eax			;edx = XTADD

	mov	ax,WORD PTR gs:FILLMIN+4[bx] ;XTstart
	mov	bx,WORD PTR gs:FILLMIN+6[bx] ;YTstart
	shl	eax,16
	shl	ebx,16

	cmp	esi,NGXLO32		;need to clip to left?
	jge	@F
	sub	esi,NGXLO32
	neg	esi

	mov	ecx,esi
	imul	ecx,edx
	add	eax,ecx			;XT+= dx*XTADD
	mov	ecx,esi
	imul	ecx,ebp
	add	ebx,ecx			;YT+= dx*YTADD
	mov	esi,NGXLO32
@@:
;eax= xtstart, ebx= YTstart, edx= XTADD ebp= YTADD ESI= xlo
	pop	ecx		;cx= xhi
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,si
	inc	cx		;cx= count
	and	ecx,0ffffh		;clear out top 16 bits
	add	di,si		;dest address start

	mov	esi,ebx		;esi= YTstart

	shl	eax,8
	shl	esi,8
	shl	edx,8
	shl	ebp,8

	pop	bx
	mov	al,bh		;color offset byte from hi of dims
        and     bl,0fch         ;check for inc pix type fills
        jne     L424


        cmp     cx,8
        jl      L422

        push    cx
        shr     cx,2
L421:
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:1[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:2[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:3[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	add     di,4
	dec	cx
	jne	L421
        pop     cx
        and     cx,3
	je	L33
L422:
        shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	inc     di
        loop    L422
        jmp     L33
L424:

        add     edx,edx                ;for inc pixel trick
        add     ebp,ebp

        cmp     cx,8
        jb      L321
        push    cx
        shr     cx,2

L32:
	shld	ebx,esi,8
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di],bl
        inc     bl
        mov     es:[di+1],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di+2],bl
        inc     bl
        mov     es:[di+3],bl
@@:
        add	eax,edx
	add	esi,ebp
	add	di,4
	loop	L32
        pop     cx
        and     cx,3
        je      L33

L321:
	shld	ebx,esi,8
	shld	ebx,eax,8
	mov	bl,fs:[bx]
	add	bl,al
	mov	es:[di],bl
        inc     di
        loop    L321

L33:
	pop	ebp
	pop	dx
	pop	bx
	add	bx,8
	add	DEST,320
	dec	dx
	jne	L30

DONE:
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret

NPFFill	ENDP
endif

NPFFill	PROC	uses di si es fs gs,DEST:DWORD,DIMS:WORD,SRC:DWORD

        mov     ax,SEG _FLLTBL
        mov     gs,ax

	mov	esi,FILLYLO
	cmp	esi,FILLYHI
	jg	DONE
	cmp	si,07fffh
	je	DONE		;no lines to fill
	mov	bx,si
	mov	edx,FILLYHI
	sub	dx,bx
	js	DONE		;bad-ylo>yhi
	inc	dx		;dx= # lines 1 based
	shl	esi,6
	lea	esi,gs:[esi+4*esi]
	add	DEST,esi	;DEST= yline screen addr to start
	shl	bx,3		;bx= start line *8 for minmaxtbls
	mov	cx,DIMS
	and	cx,3
	cmp	cx,2
	je	L3
	cmp	cx,1		;Gouroud-1dim fill
	je	L2

L1:
	les	si,DEST
	mov	eax,SRC
	push	bp
	mov	bp,ax

        ; do special type fills here
        cmp     ah,250                 ;transparent xlat fill
        jne     L10



L10a:
	mov	eax,gs:FILLMIN[bx]
        mov     gs:FILLMIN[bx],07fffffffh
	sar	eax,XBITS
	mov	ecx,gs:FILLMAX[bx]
        mov     gs:FILLMAX[bx],0ffffffffh
	sar	ecx,XBITS
	cmp	ecx,eax
	jge	@F

	jmp	L11a                     ;vert case fix

	xchg	eax,ecx
@@:
	cmp	eax,NGXHI32
	jg	L11a
	cmp	ecx,NGXLO32
	jl	L11a
	cmp	eax,NGXLO32
	jge	@F
	mov	eax,NGXLO32
@@:
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,ax		;cx= #pixels
	inc	cx		;ax= start pix
	mov	di,si
	add	di,ax

        push    bx
        mov     bx,NTRANSGREYPTRS

@@:
	mov	al,es:[di]
	xlat
	stosb
	loop	@B
        pop     bx
L11a:
	add	bx,8
	add	si,320
	dec	dx
	jne	L10a
	pop	bp
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret


L10:
	mov	eax,gs:FILLMIN[bx]
        mov     gs:FILLMIN[bx],07fffffffh
	sar	eax,XBITS
	mov	ecx,gs:FILLMAX[bx]
        mov     gs:FILLMAX[bx],0ffffffffh
	sar	ecx,XBITS
	cmp	ecx,eax
	jge	@F

	jmp	L11                     ;vert case fix

	xchg	eax,ecx
@@:
	cmp	eax,NGXHI32
	jg	L11
	cmp	ecx,NGXLO32
	jl	L11
	cmp	eax,NGXLO32
	jge	@F
	mov	eax,NGXLO32
@@:
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,ax		;cx= #pixels
	inc	cx		;ax= start pix
	mov	di,si
	add	di,ax
	mov	ax,bp		;color word
	test	bx,08h		;line <<3
	je	@F
	xchg	al,ah
@@:
	test	di,1
	je	@F
	mov	es:[di],ah
	inc	di
	dec	cx
	je	L11
@@:
        cmp     cx,8
        jb      @F
        push    ax
        shl     eax,16
        pop     ax
        push    cx
        shr     cx,2
        rep     stosd
        pop     cx
        and     cx,3
@@:
	shr	cx,1
	je	@F
	rep	stosw
@@:
	jnc	L11
	mov	es:[di],al
L11:
	add	bx,8
	add	si,320
	dec	dx
	jne	L10
	pop	bp
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret
L2:
;1 dim fill -gouroud fill
;DEST= start of top yline
;bx=8*y line # (for min,max tbls)
;dx= # lines to do 1 based


	mov	CNT,dx


L20:
	les	di,DEST
	mov	esi,gs:FILLMIN[bx]
        mov     gs:FILLMIN[bx],07fffffffh
	sar	esi,XBITS
	mov	ecx,gs:FILLMAX[bx]
        mov     gs:FILLMAX[bx],0ffffffffh
	sar	ecx,XBITS
	cmp	ecx,esi
	jge	@F

	jmp	L21                     ;vert case fix

	xchg	esi,ecx
@@:
	cmp	esi,NGXHI32
	jg	L21
	cmp	ecx,NGXLO32
	jl	L21

;	jmp	L21		;!!!!!!!!!!!!!!!!!!!!!!!!

	push	ecx
	sub	ecx,esi
	inc	ecx

	mov	ax,WORD PTR gs:FILLMAX+4[bx]
	sub	ax,WORD PTR gs:FILLMIN+4[bx]
	shl	eax,16
	cdq
	idiv	ecx			;eax = xtadd<<16
	mov	ecx,eax			;ecx= xtadd<<16
	mov	dx,WORD PTR gs:FILLMIN+4[bx]
	shl	edx,16			;edx= xtstart
	cmp	esi,NGXLO32		;need to clip to left?
	jge	@F
	mov	eax,NGXLO32
	xchg	eax,esi			;esi=NGXLO32, eax= old xval
	sub	eax,esi
	neg	eax
	imul	eax,ecx			;dx*xtadd
	add	edx,eax			;xt+= dx*xtadd
@@:
;edx= xtstart, ecx= xtadd, esi=xstart
	add	di,si
	mov	ax,si			;ax= xlo
	mov	esi,ecx			;esi= xtadd
	shl	edx,8
	shl	esi,8
	pop	ecx			;xhi
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,ax			;cx= end-start
	inc	cx
	and	ecx,0ffffh		;clear out top 16 bits
@@:
	test	di,1
	je	@F
	shld	eax,edx,8
	add	edx,esi
	mov	es:[di],al
	inc	di
	dec	cx
	je	L21
@@:
	ror	ecx,3			;save 0-3 bit in 31-29 bit for later
	jcxz	@F
L22:
	shld	eax,edx,8
        mov     es:[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:1[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:2[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:3[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:4[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:5[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:6[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:7[di],al
	add	edx,esi
        add	di,8
	dec	cx
	jne	L22
@@:
	shl	ecx,1
	jnc	@F
	shld	eax,edx,8
        mov     es:[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:1[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:2[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:3[di],al
	add	edx,esi
	add	di,4
@@:
	shl	ecx,1
	jnc	@F
	shld	eax,edx,8
        mov     es:[di],al
	add	edx,esi
	shld	eax,edx,8
        mov     es:1[di],al
	add	edx,esi
	add	di,2
@@:
	shl	ecx,1
	jnc	L21
	shld	eax,edx,8
	mov	es:[di],al
;	add	edx,esi
L21:
	add	bx,8
	add	DEST,320
	dec	CNT
	jne	L20
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret
;2 dim fill
L3:
	lfs	si,SRC
	shr	si,4
	mov	ax,fs
	add	ax,si
	mov	fs,ax
L30:
	push	bx
	push	dx
	push	ebp
L31:
	les	di,DEST
	mov	esi,gs:FILLMIN[bx]
        mov     gs:FILLMIN[bx],07fffffffh
	sar	esi,XBITS
	mov	ecx,gs:FILLMAX[bx]
        mov     gs:FILLMAX[bx],0ffffffffh
	sar	ecx,XBITS
	cmp	ecx,esi
	jge	@F

	jmp	L33                     ;vert case fix

	xchg	esi,ecx
@@:
	cmp	esi,NGXHI32
	jg	L33
	cmp	ecx,NGXLO32
	jl	L33


;	jmp	L33			;!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


	push	DIMS

	push	ecx
	sub	ecx,esi
	inc	ecx
	mov	ax,WORD PTR gs:FILLMAX+6[bx]
	sub	ax,WORD PTR gs:FILLMIN+6[bx]
	shl	eax,16
	cdq
	idiv	ecx			;eax = ytadd<<16
	mov	ebp,eax			;ebp = YTADD
	mov	ax,WORD PTR gs:FILLMAX+4[bx]
	sub	ax,WORD PTR gs:FILLMIN+4[bx]
	shl	eax,16
	cdq
	idiv	ecx			;eax = xtadd<<16
	mov	edx,eax			;edx = XTADD

	mov	ax,WORD PTR gs:FILLMIN+4[bx] ;XTstart
	mov	bx,WORD PTR gs:FILLMIN+6[bx] ;YTstart
	shl	eax,16
	shl	ebx,16

	cmp	esi,NGXLO32		;need to clip to left?
	jge	@F
	sub	esi,NGXLO32
	neg	esi

	mov	ecx,esi
	imul	ecx,edx
	add	eax,ecx			;XT+= dx*XTADD
	mov	ecx,esi
	imul	ecx,ebp
	add	ebx,ecx			;YT+= dx*YTADD
	mov	esi,NGXLO32
@@:
;eax= xtstart, ebx= YTstart, edx= XTADD ebp= YTADD ESI= xlo
	pop	ecx		;cx= xhi
	cmp	ecx,NGXHI32
	jle	@F
	mov	ecx,NGXHI32
@@:
	sub	cx,si
	inc	cx		;cx= count
	and	ecx,0ffffh		;clear out top 16 bits
	add	di,si		;dest address start

	mov	esi,ebx		;esi= YTstart

	shl	eax,8
	shl	esi,8
	shl	edx,8
	shl	ebp,8

	pop	bx
	mov	al,bh		;color offset byte from hi of dims
        and     bl,0fch         ;check for inc pix type fills
        jne     L424


        cmp     cx,8
        jl      L422

        push    cx
        shr     cx,2
L421:
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:1[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:2[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:3[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	add     di,4
	dec	cx
	jne	L421
        pop     cx
        and     cx,3
	je	L33
L422:
        shld	ebx,esi,8       ;this is temporary for single pix resolution
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di],bl
@@:
        add	eax,edx
	add	esi,ebp
	inc     di
        loop    L422
        jmp     L33
L424:

        add     edx,edx                ;for inc pixel trick
        add     ebp,ebp

        cmp     cx,8
        jb      L321
        push    cx
        shr     cx,2

L32:
	shld	ebx,esi,8
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di],bl
        inc     bl
        mov     es:[di+1],bl
@@:
        add	eax,edx
	add	esi,ebp
	shld	ebx,esi,8
	shld	ebx,eax,8
	mov	bl,fs:[bx]
        cmp     bl,0
        je      @F
	add	bl,al
	mov	es:[di+2],bl
        inc     bl
        mov     es:[di+3],bl
@@:
        add	eax,edx
	add	esi,ebp
	add	di,4
	loop	L32
        pop     cx
        and     cx,3
        je      L33

L321:
	shld	ebx,esi,8
	shld	ebx,eax,8
	mov	bl,fs:[bx]
	add	bl,al
	mov	es:[di],bl
        inc     di
        loop    L321

L33:
	pop	ebp
	pop	dx
	pop	bx
	add	bx,8
	add	DEST,320
	dec	dx
	jne	L30

DONE:
	mov	FILLYLO,07fffh
	mov	FILLYHI,-1
	ret

NPFFill	ENDP



MulDiv32	PROC   M1:DWORD,M2:DWORD,DV:DWORD
	mov	eax,M1
	imul	M2
	idiv	DV
	shld	edx,eax,16	;return in dx:ax
	ret

MulDiv32	ENDP

LongDiv	PROC	N:DWORD,D:DWORD
	mov	eax,N
	cdq
	idiv	D
	shld	edx,eax,16	;return in dx:ax
	ret
LongDiv	ENDP


.8086



NSetViewMat	PROC	MAT:WORD,ZOOM:WORD
;
;ZOOM	100h= lowest zoom
;
	mov	ax,ZOOM

	shl	ax,1

	ABSAX
	cmp	ax,100h
	jae	@F
   	mov	ax,100h
@@:
	mov	cx,ax
	mov	dx,07fh
	mov	ax,0ff00h
	div	cx
	mov	zoomfactor,ax		;save it
;ax= zoomfactor to "mult" z coords by
	lea	ax,Nviewmat
	push	ax
	mov	ax,MAT
	push	ax
	call	Copy3X3
	add	sp,4
	lea	ax,Nviewmat+18
	push	ax
	sub	ax,18
	push	ax
	call	Trans3X3
;fix up view mat for neg z and zoom factor !!!!	AND for yscreen + = down !!!

	lea	bx,Nviewmat
	mov	ax,zoomfactor
	neg	ax
	mov	cx,ax
	SMUL	[bx].R1.C3
	mov	[bx].R1.C3,dx
	mov	ax,cx
	SMUL	[bx].R2.C3
	mov	[bx].R2.C3,dx
	mov	ax,cx
	SMUL	[bx].R3.C3
	mov	[bx].R3.C3,dx


; fisheye things here

	sar	WORD PTR [bx].R1.C1,1
	sar	WORD PTR [bx].R2.C1,1
	sar	WORD PTR [bx].R3.C1,1

	sar	WORD PTR [bx].R1.C2,1
	sar	WORD PTR [bx].R2.C2,1
	sar	WORD PTR [bx].R3.C2,1





;y screen fix
	cmp	NOASPECT,0
	je	@F
	neg	WORD PTR [bx].R1.C2
	neg	WORD PTR [bx].R2.C2
	neg	WORD PTR [bx].R3.C2
	jmp	svm_out
@@:
	mov	ax,[bx].R1.C2
	neg	ax
	mov	cx,ax
	sar	cx,1
	sar	cx,1
	sub	ax,cx
	mov	[bx].R1.C2,ax

	mov	ax,[bx].R2.C2
	neg	ax
	mov	cx,ax
	sar	cx,1
	sar	cx,1
	sub	ax,cx
	mov	[bx].R2.C2,ax

	mov	ax,[bx].R3.C2
	neg	ax
	mov	cx,ax
	sar	cx,1
	sar	cx,1
	sub	ax,cx
	mov	[bx].R3.C2,ax

svm_out:
	add	sp,4
	ret
NSetViewMat	ENDP

.386
fmemfill        PROC    uses es di, DST:DWORD,VAL:WORD, LEN:WORD
        mov     ax,VAL
        shl     eax,16
        mov     ax,VAL

	mov	cx,LEN
	jcxz	L1
	les	di,DST
        cmp     cx,4
        jb      L2
        push    cx
        shr     cx,2
        rep     stosd
        pop     cx
        and     cx,3
L2:
        or      cx,cx
        je L1
	rep	stosb
L1:
	ret

fmemfill        ENDP

FillScreen      PROC    uses es di,DST:WORD,VAL:WORD
        mov     cx,320*50
        mov     ax,VAL
        shl     eax,16
        mov     ax,VAL
        mov     es,DST
        sub     di,di

        rep     stosd
        ret

FillScreen      ENDP

fmemcpy	PROC	uses es ds si di cx,SRC:DWORD,DST:DWORD,LEN:WORD

	mov	cx,LEN
	jcxz	L1
	lds	si,SRC
	les	di,DST

        cmp     cx,4
        jb      L2
        push    cx
        shr     cx,2
        rep     movsd
        pop     cx
        and     cx,3
L2:
        or      cx,cx
        je L1
	rep	movsb
L1:
	ret

fmemcpy	ENDP

.8086


.386
DotProduct      PROC    P1:WORD,P2:WORD,P3:WORD,P4:WORD,P5:WORD,P6:WORD

        movsx   eax,P1
        movsx   edx,P4
        imul    eax,edx
        mov     ecx,eax

        movsx   eax,P2
        movsx   edx,P5
        imul    eax,edx
        add     ecx,eax

        movsx   eax,P3
        movsx   edx,P6
        imul    eax,edx
        add     ecx,eax

        mov     eax,ecx
        shld    edx,eax,16
        ret
DotProduct      ENDP


.8086


if 0



ScaleLine	PROC	USES si di bx ds es,SRCSEG:WORD,BASE:WORD,ADDR:WORD,X:WORD,Y:WORD,W:WORD,DEST:WORD

	mov	ds,SRCSEG
	mov	si,BASE
	mov	ax,DEST
	mov	es,ax

	mov	ax,Y
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	bx,ax
	shl	ax,1
	shl	ax,1
	add	ax,bx
	mov	di,ax
	add	di,X

	mov	cx,W
	mov	bp,ADDR
	shr	cx,1
	shr	cx,1

	shr	cx,1

@@:
	mov	bx,[bp]
	mov	al,[bx+si]
	mov	es:[di],al

	mov	bx,2[bp]
	mov	al,[bx+si]
	mov	es:1[di],al

	mov	bx,4[bp]
	mov	al,[bx+si]
	mov	es:2[di],al

	mov	bx,6[bp]
	mov	al,[bx+si]
	mov	es:3[di],al

	mov	bx,8[bp]
	mov	al,[bx+si]
	mov	es:4[di],al

	mov	bx,10[bp]
	mov	al,[bx+si]
	mov	es:5[di],al

	mov	bx,12[bp]
	mov	al,[bx+si]
	mov	es:6[di],al

	mov	bx,14[bp]
	mov	al,[bx+si]
	mov	es:7[di],al


	add	bp,16
	add	di,8
	loop	@B

	ret
ScaleLine	ENDP

ScaleLine2	PROC	USES si di bx ds es,SRCSEG:WORD,BASE:WORD,ADDR:WORD,X:WORD,Y:WORD,W:WORD,DEST:WORD

	mov	ds,SRCSEG
	mov	si,BASE
	mov	ax,DEST
	mov	es,ax

	mov	ax,Y
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	bx,ax
	shl	ax,1
	shl	ax,1
	add	ax,bx
	mov	di,ax
	add	di,X

	mov	cx,W
	mov	bp,ADDR
	shr	cx,1
	shr	cx,1

	shr	cx,1

@@:
	mov	bx,[bp]
	mov	al,[bx+si]
        mov     ah,al
        inc     ah
        mov	es:[di],ax

	mov	bx,4[bp]
	mov	al,[bx+si]
        mov     ah,al
        inc     ah
        mov	es:2[di],ax

	mov	bx,8[bp]
	mov	al,[bx+si]
        mov     ah,al
        inc     ah
        mov	es:4[di],ax

	mov	bx,12[bp]
	mov	al,[bx+si]
        mov     ah,al
        inc     ah
        mov	es:6[di],ax


	add	bp,16
	add	di,8
	loop	@B

	ret
ScaleLine2	ENDP

XSLine	PROC	USES	bx cx, A:DWORD,E:WORD,LN:WORD,HT:WORD,M:WORD

.386
	mov	ax,M
	cwde
	mov	ecx,eax
	mov	ax,HT
	cwde
	mov	ebx,eax
	mov	ax,LN
	cwde
	mov	edx,eax
	mov	ax,E
	cwde

	imul	edx
	add	eax,A
	cdq
	idiv	ecx
	imul	ebx

	shld	edx,eax,16

.8086
	ret

XSLine	ENDP


XSADDLine	PROC	uses bx cx, C:WORD,HT:WORD,M:WORD

.386
	mov	ax,M
	cwde
	mov	ecx,eax
	mov	ax,HT
	cwde
	mov	ebx,eax
	mov	ax,C
	cwde
	imul	ebx
	idiv	ecx

	shld	edx,eax,16

.8086
	ret



XSADDLine	ENDP


CloudLine	PROC	USES bp si di bx ds es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR:WORD,DST:WORD,GREY:WORD,WID:WORD
        LOCAL   LCNT:WORD


	mov	ds,SRCSEG
	mov	di,ADDR
	mov	ax,DST
	mov	es,ax



    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	ax,ZADD


	shl	si,1
	shl	ax,1
	shl	si,1
	shl	ax,1


	shl	cx,1			;for pixel double ?????

	mov	bx,WID
	shr	bx,1
	shr	bx,1
	mov	LCNT,bx

.386

L1:
	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
	mov	es:[di],bl
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
	mov	es:1[di],bl
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
	mov	es:2[di],bl
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
	mov	es:3[di],bl
	add	cx,si
	add	dx,ax


.8086
	add	di,4
	dec	LCNT
	jne	L1
	ret

CloudLine	ENDP

CloudLine2	PROC	USES bp si di bx ds es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR:WORD,DST:WORD,GREY:WORD,WID:WORD
        LOCAL   LCNT:WORD


	mov	ds,SRCSEG
	mov	di,ADDR
	mov	ax,DST
	mov	es,ax



    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	ax,ZADD


	shl	si,1
	shl	ax,1
	shl	si,1
	shl	ax,1


	shl	cx,1			;for pixel double ?????

	mov	bx,WID
	shr	bx,1
	shr	bx,1
	shr	bx,1
	mov	LCNT,bx

	shl	si,1		;double
	shl	ax,1		;double


.386

L1:
	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
        mov     bh,bl
;        inc     bh
	mov	es:[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
        mov     bh,bl
;        inc     bh
	mov	es:2[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
        mov     bh,bl
;        inc     bh
	mov	es:4[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	shr	bx,1
	and	bx,3fffh
	mov	bl,[bx]
        add     bl,BYTE PTR GREY
        mov     bh,bl
;        inc     bh
	mov	es:6[di],bx
	add	cx,si
	add	dx,ax


.8086
	add	di,8
	dec	LCNT
	jne	L1
	ret

CloudLine2	ENDP




DrawFromOFFTBL  PROC    USES es ds si di bp,CT:WORD,PTRS:WORD,SRC:DWORD,DST:DWORD

        mov     cx,CT
        lds     si,SRC
        les     di,DST
        mov     bp,PTRS

@@:
        mov     bx,ss:[bp]         ;get offset val
        mov     al,[bx+si]      ;get source byte
        mov     es:[bx+di],al      ;store byte at dest+offset
        add     bp,2
        loop    @B
        ret

DrawFromOFFTBL  ENDP

endif

.8086

NScaleLine	PROC	USES si di bx ds es,SRCSEG:WORD,BASE:WORD,ADDR2:WORD,X:WORD,Y:WORD,W:WORD,DEST:WORD

	mov	ds,SRCSEG
	mov	si,BASE
	mov	ax,DEST
	mov	es,ax

	mov	ax,Y
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	bx,ax
	shl	ax,1
	shl	ax,1
	add	ax,bx
	mov	di,ax
	add	di,X

	mov	cx,W
	mov	bp,ADDR2
	shr	cx,1
	shr	cx,1

	shr	cx,1

@@:
	mov	bx,[bp]
	mov	al,[bx+si]
	mov	bx,2[bp]
	mov	ah,[bx+si]
	mov	es:[di],ax

	mov	bx,4[bp]
	mov	al,[bx+si]
	mov	bx,6[bp]
	mov	ah,[bx+si]
	mov	es:2[di],ax

	mov	bx,8[bp]
	mov	al,[bx+si]
	mov	bx,10[bp]
	mov	ah,[bx+si]
	mov	es:4[di],ax

	mov	bx,12[bp]
	mov	al,[bx+si]
	mov	bx,14[bp]
	mov	ah,[bx+si]
	mov	es:6[di],ax

        add	bp,16
	add	di,8
	loop	@B

	ret
NScaleLine	ENDP

NScaleLine2	PROC	USES si di bx ds es,SRCSEG:WORD,BASE:WORD,ADDR2:WORD,X:WORD,Y:WORD,W:WORD,DEST:WORD

	mov	ds,SRCSEG
	mov	si,BASE
	mov	ax,DEST
	mov	es,ax

	mov	ax,Y
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	shl	ax,1
	mov	bx,ax
	shl	ax,1
	shl	ax,1
	add	ax,bx
	mov	di,ax
	add	di,X

	mov	cx,W
	mov	bp,ADDR2
	shr	cx,1
	shr	cx,1

	shr	cx,1

@@:
	mov	bx,[bp]
	mov	al,[bx+si]
	mov	es:[di],al

        inc     al
        mov	es:1[di],al

	mov	bx,4[bp]
	mov	al,[bx+si]
	mov	es:2[di],al

        inc     al
        mov	es:3[di],al

	mov	bx,8[bp]
	mov	al,[bx+si]
	mov	es:4[di],al

        inc     al
        mov	es:5[di],al

	mov	bx,12[bp]
	mov	al,[bx+si]
	mov	es:6[di],al

        inc     al
        mov	es:7[di],al


	add	bp,16
	add	di,8
	loop	@B

	ret
NScaleLine2	ENDP

LCNT	dw	1


NCloudLine	PROC	USES bp si di bx ds es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR2:WORD,DST:WORD,GREY:WORD,WID:WORD



	mov	ds,SRCSEG
	mov	di,ADDR2
	mov	ax,DST
	mov	es,ax



    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	ax,ZADD


	shl	si,1
	shl	ax,1
;	shl	si,1
;	shl	ax,1


;	shl	cx,1			;for pixel double ?????

	mov	bx,WID
	shr	bx,1
	shr	bx,1
	mov	bp,bx

.386

L1:
	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
	mov	es:[di],bl
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
	mov	es:1[di],bl
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
	mov	es:2[di],bl
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
	mov	es:3[di],bl
	add	cx,si
	add	dx,ax


.8086
	add	di,4
	dec	bp
	jne	L1
	ret

NCloudLine	ENDP

NCloudLine2	PROC	USES bp si di bx ds es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR2:WORD,DST:WORD,GREY:WORD,WID:WORD



	mov	ds,SRCSEG
	mov	di,ADDR2
	mov	ax,DST
	mov	es,ax



    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	ax,ZADD


	shl	si,1
	shl	ax,1
	shl	si,1
	shl	ax,1


;	shl	cx,1			;for pixel double ?????


	mov	bx,WID
	shr	bx,1
	shr	bx,1
	shr	bx,1
	mov	bp,bx


.386

L1:
	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
        mov     bh,bl
	mov	es:[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
        mov     bh,bl
	mov	es:2[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
        mov     bh,bl
	mov	es:4[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
        mov     bh,bl
	mov	es:6[di],bx
	add	cx,si
	add	dx,ax


.8086
	add	di,8
	dec	bp
	jne	L1
	ret

NCloudLine2	ENDP

NCloudLine3	PROC	USES bp si di bx ds es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR2:WORD,DST:WORD,GREY:WORD,WID:WORD



	mov	ds,SRCSEG
	mov	di,ADDR2
	mov	ax,DST
	mov	es,ax



    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	ax,ZADD


	shl	si,1
	shl	ax,1
	shl	si,1
	shl	ax,1


	shl	si,1
	shl	ax,1


	mov	bx,WID
	shr	bx,1
	shr	bx,1
	shr	bx,1
	mov	bp,bx


.386

L1:
	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
        mov     bh,bl
	mov	es:[di],bx
	mov	es:2[di],bx
	add	cx,si
	add	dx,ax

	mov	bh,dh
	mov	bl,ch
	and	bx,7f7fh
	mov	bl,[bx]
        mov     bh,bl
	mov	es:4[di],bx
	mov	es:6[di],bx
	add	cx,si
	add	dx,ax


.8086
	add	di,8
	dec	bp
	jne	L1
	ret

NCloudLine3	ENDP



NXSLine	PROC	USES	bx cx, A:DWORD,E:WORD,LN:WORD,HT:WORD,M:WORD

.386
	mov	ax,M
	cwde
	mov	ecx,eax
	mov	ax,HT
	cwde
	mov	ebx,eax
	mov	ax,LN
	cwde
	mov	edx,eax
	mov	ax,E
	cwde

	imul	edx
	add	eax,A
	cdq
	idiv	ecx
	imul	ebx

	shld	edx,eax,16

.8086
	ret

NXSLine	ENDP


NXSADDLine	PROC	uses bx cx, C_2:WORD,HT:WORD,M:WORD

.386
	mov	ax,M
	cwde
	mov	ecx,eax
	mov	ax,HT
	cwde
	mov	ebx,eax
	mov	ax,C_2
	cwde
	imul	ebx
	idiv	ecx

	shld	edx,eax,16

.8086
	ret



NXSADDLine	ENDP




NCloudFillLine2	PROC	USES bp si di bx ds es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR_2:WORD,DST:WORD,GREY:WORD,WID:WORD



	mov	di,ADDR_2
	mov	ax,DST
	mov	es,ax




	mov	cx,WID
	shr	cx,1

.386
	mov	fs,GREY

	mov	bl,08ch
	sub	bh,bh
	mov	al,fs:[bx]
	mov	ah,al

	rep	stosw

.8086
	ret

NCloudFillLine2	ENDP

.386

NDLine	PROC	uses di si es fs gs,P1:DWORD,P2:DWORD,DST:DWORD,CLR:WORD


P1X	equ	es:[di]
P1Y	equ	es:4[di]
P1Z	equ	es:8[di]
P1XT	equ	es:12[di]
P1YT	equ	es:14[di]
P2X	equ	fs:[si]
P2Y	equ	fs:4[si]
P2Z	equ	fs:8[si]
P2XT	equ	fs:12[si]
P2YT	equ	fs:14[si]

	push	bp

	les	di,P1
	lfs	si,P2
	mov	eax,P2Y
	mov	ecx,P1Y
	cmp	eax,ecx		;bottom >= top
	jg	@F
	les	di,P2
	lfs	si,P1
	xchg	ecx,eax
@@:
	cmp	eax,NGYLO32		;y2< top ?
	jl	DONE
	cmp	ecx,NGYHI32		;y1 >  bottom?
	jg	DONE

	sub	eax,ecx		;p2y-py1


	inc	eax		;+1 = delta y
	mov	ebx,eax		;save in ebx
L1:
	mov	eax,P2X
	mov	edx,P1X
	sub	eax,edx
	shl	eax,XBITS	;shift up for precision
	shl	edx,XBITS
	mov	NPX,edx
	mov	edx,1           ;put back
	shl	edx,XBITS-1     ; why -1 ???????
	add	NPX,edx
	cdq
	idiv	ebx		;delx /dely= add val
	mov	NPXADD,eax

	mov	eax,P1Y		;get lo fill y
	mov	ecx,eax		;cx = ylo
	sub	eax,NGYLO32
	jge	@F

	neg	eax
	mov	ebx,eax 		;dy

	mov	eax,ebx		;cx= dy*NPYTADD
	imul	eax,NPYTADD
	add	NPYT,eax

	mov	eax,ebx		;cx= dy*NPXTADD
	imul	eax,NPXTADD
	add	NPXT,eax

	mov	eax,ebx		;cx= dy*NPXADD
	imul	eax,NPXADD
	add	NPX,eax

	mov	ecx,NGYLO32		;new ylo
@@:
	mov	eax,P2Y
	cmp	eax,NGYHI32
	jle	@F
	mov	eax,NGYHI32		;new yhi
@@:

	mov	esi,ecx
        shl     esi,6                   ;*64
        les     bx,DST

        lea     esi,es:[esi+4*esi]      ;64+256
        add     si,bx
	sub	ecx,eax
	neg	cx
	inc	cx
	mov	dx,cx	 	;dx= # of lines to do 1 based, si= tbl base
	mov	ebx,NPX
        mov     ax,CLR
L4:
        mov     edi,ebx
        add     ebx,NPXADD
        mov     ecx,ebx
	sar	edi,XBITS
        sar     ecx,XBITS
        cmp     edi,ecx
        jle     @F
        xchg    ecx,edi
@@:
        cmp     edi,NGXLO32
        jge     @F
        mov     edi,NGXLO32
@@:
        cmp     ecx,NGXHI32
        jle     @F
        mov     ecx,NGXHI32
@@:
        sub     ecx,edi
        js      @F
        inc     cx
        add     di,si
        rep     stosb
@@:
        add     si,320
	dec	dx		;put more iterations to amortizes the loop !!
	jne	L4
DONE:
	pop	bp
	ret
NDLine	ENDP

NDDot	PROC	uses di si es fs gs,P1:DWORD,DST:DWORD,CLR:WORD


P1X	equ	es:[di]
P1Y	equ	es:4[di]
P1Z	equ	es:8[di]
P1XT	equ	es:12[di]
P1YT	equ	es:14[di]
P2X	equ	fs:[si]
P2Y	equ	fs:4[si]
P2Z	equ	fs:8[si]
P2XT	equ	fs:12[si]
P2YT	equ	fs:14[si]

	push	bp

	les	di,P1
	mov	esi,P1Y
	cmp	esi,NGYLO32		;y2< top ?
	jl	DONE
	cmp	esi,NGYHI32		;y1 >  bottom?
	jg	DONE
        mov     edx,P1X
        cmp     edx,NGXLO32
        jl      DONE
        cmp     edx,NGXHI32
        jg      DONE

        shl     esi,6                   ;*64
        les     bx,DST

        lea     esi,es:[esi+4*esi]      ;64+256
        add     si,bx
        add     si,dx
        mov     ax,CLR

        mov     es:[si],al
DONE:
	pop	bp
	ret
NDDot	ENDP



;struct objptrs{
;0   void far *OBJBASE;
;4   unsigned int far *GOPBASE;
;8   struct   ptdef far *PTSDEFBASE;
;12   struct	PT3D far *PTSROT;
;16   char far *TXTR;
;20   struct   obj3d *OBJDEF;
;22   long  RELADD[5][3];
;82   int   *MATPTR;
;84   int  *ANIM;
;86   struct intvec *OFFS;
;88   unsigned char ORDER[256];
;344   int   dumppts[5][3];
;374   int   MATS[5][18];
;   };
;struct   obj3d {
;   int   NUM;
;   int   XR;
;   int   YR;
;   int   ZR;
;   int   *MAT;
;   int   *ANIM;
;   long  RELADD[3];
;   };
; struct	PT3D{
;0	long	XR;
;4	long	YR;
;8	long	ZR;
;12	long	XP;
;16	long	YP;
;20	long	ZP;
;24	int	XT;
;26	int	YT;
;28	int	ZT;
;30	int	FLAGS;
;	};
;struct   ptdef{
;   int   x;
;   int   y;
;   int   z;
;   int   mat;
;   };
;

if 1
.386
CheckPointRot   PROC    uses es fs gs si di,PT:WORD

PTDFBS  = 8
PTSROT  = 12
RLADD   = 22
MTPTR   = 82
ANMPTR  = 84
PFLGS   = 30
RDFLAG	= 64

P       equ     es:[si]
OBJPTR     equ     [di]
PDEF    equ     gs:[bx]

        mov     di,OBJ
        les     si,OBJPTR.PTSROT
        mov     ax,PT
        and     ax,0ffh
        mov     cx,ax
        shl     ax,5
        add     si,ax           ;P= rot point area
        mov     ax,P.PFLGS
        and     ax,RDFLAG
        jne     done
        lgs     bx,OBJPTR.PTDFBS
        mov     ax,cx
        shl     ax,3
        add     bx,ax           ;PDEF= point definition
        mov     eax,PDEF
        mov     DWORD PTR VECT,eax
        mov     ax,PDEF+4
        mov     VECT+4,ax
        mov     ax,PDEF.6
        mov     cx,ax
        test    ax,0fff0h
        je      @F
        mov     bx,OBJPTR.ANMPTR
        shr     ax,3
        and     ax,0fffeh
        add     bx,ax           ;ds:bx= pointer to anim vals
        mov     ax,[bx]         ;add x,y,z offsets to point
        add     VECT,ax
        mov     ax,2[bx]
        add     VECT+2,ax
        mov     ax,4[bx]
        add     VECT+4,ax
        and     cx,0fh
@@:
        push    cx

        lea     ax,LVECT
        push    ax
        mov     ax,cx           ;cx *36 = (*8 +1)*4
        shl     ax,3
        add     ax,cx
        shl     ax,2
        mov     bx,OBJPTR.MTPTR
        add     ax,bx
        push    ax
        lea     ax,VECT
        push    ax
        call    Mult1X3
        add     sp,6

        pop     cx              ;m

        mov     ax,cx
        shl     ax,1
        add     ax,cx           ;cx*3
        shl     ax,2           ;m*12
        mov     bx,di
        add     bx,RLADD        ; bx points to obj.reladd[0][0]
        add     bx,ax           ;bx points to obj.relladd[m][0]

        mov     eax,[bx]
        add     eax,LVECT
        mov     P,eax
        mov     eax,[bx].4
        add     eax,LVECT+4
        mov     P.4,eax
        mov     eax,[bx].8
        add     eax,LVECT+8
        mov     P.8,eax
        
        push    es
        push    si
        call    FlagRotate3D
        pop     ax
        pop     dx
        ret
done:
        mov     ax,si
        mov     dx,es
        ret
CheckPointRot   ENDP
.8086
endif
if 0
.386
CheckPointRot   PROC    uses es fs gs si di,PT:WORD

PTDFBS  = 8
PTSROT  = 12
RLADD   = 22
MTPTR   = 82
ANMPTR  = 84
PFLGS   = 30
RDFLAG	= 64

P       equ     es:[si]
OBJPTR     equ     [di]
PDEF    equ     gs:[bx]

        mov     di,OBJ
        les     si,OBJPTR.PTSROT
        mov     ax,PT
        and     ax,0ffh
        mov     cx,ax
        shl     ax,5
        add     si,ax           ;P= rot point area
        mov     ax,P.PFLGS
        and     ax,RDFLAG
        jne     done
        lgs     bx,OBJPTR.PTDFBS
        mov     ax,cx
        shl     ax,3
        add     bx,ax           ;PDEF= point definition

        push    ebx
        push    esi
        push    edi
        push    ebp

        movsx   ecx,WORD PTR PDEF
        movsx   edx,WORD PTR PDEF.2
        movsx   esi,WORD PTR PDEF.4

        mov     ax,PDEF.6
        test    ax,0fff0h
        je      @F
        mov     bx,OBJPTR.ANMPTR
        shr     ax,3
        and     ax,0fffeh
        add     bx,ax           ;ds:bx= pointer to anim vals
        
        movsx   eax,WORD PTR [bx]         ;add x,y,z offsets to point
        add     ecx,eax
        movsx   eax,WORD PTR [bx+2]         ;add x,y,z offsets to point
        add     edx,eax
        movsx   eax,WORD PTR [bx+4]         ;add x,y,z offsets to point
        add     esi,eax

        mov     ax,PDEF.6
        and     ax,0fh
@@:
        push    ax

        mov     bx,ax
        shl     ax,3
        add     ax,bx
        shl     ax,2
        mov     di,OBJPTR.MTPTR
        add     di,ax
        lea     bx,LVECT

        movsx   ebp,WORD PTR [di].R1.C1
        imul    ebp,ecx
        movsx   eax,WORD PTR [di].R2.C1
        imul    eax,edx
        add     ebp,eax
        movsx   eax,WORD PTR [di].R3.C1
        imul    eax,esi
        add     ebp,eax
        mov     [bx],ebp

        movsx   ebp,WORD PTR [di].R1.C2
        imul    ebp,ecx
        movsx   eax,WORD PTR [di].R2.C2
        imul    eax,edx
        add     ebp,eax
        movsx   eax,WORD PTR [di].R3.C2
        imul    eax,esi
        add     ebp,eax
        mov     [bx+4],ebp

        movsx   ebp,WORD PTR [di].R1.C3
        imul    ebp,ecx
        movsx   eax,WORD PTR [di].R2.C3
        imul    eax,edx
        add     ebp,eax
        movsx   eax,WORD PTR [di].R3.C3
        imul    eax,esi
        add     ebp,eax
        mov     [bx+8],ebp

        pop     cx              ;m

        pop     ebp
        pop     edi
        pop     esi
        pop     ebx

        mov     ax,cx
        shl     ax,1
        add     ax,cx           ;cx*3
        shl     ax,2           ;m*12
        mov     bx,di
        add     bx,RLADD        ; bx points to obj.reladd[0][0]
        add     bx,ax           ;bx points to obj.relladd[m][0]

        mov     eax,[bx]
        add     eax,LVECT
        mov     P,eax
        mov     eax,[bx].4
        add     eax,LVECT+4
        mov     P.4,eax
        mov     eax,[bx].8
        add     eax,LVECT+8
        mov     P.8,eax
        
        push    es
        push    si
        call    FlagRotate3D
        pop     ax
        pop     dx
        ret
done:
        mov     ax,si
        mov     dx,es
        ret
CheckPointRot   ENDP
.8086
endif

BspSrt:
; cx = node
;es:[si]= bspbase
;[di]= drawptr
;dx= fbptr

        cmp     cl,255
        jne     @F
        mov     [di],cl
        retn
@@:
        mov     bx,dx           ; if( (*(FBPTR+node))&1 )
        add     bx,cx
        mov     al,[bx]
        and     al,1
        je      bspback
        mov     bx,cx           ;MakeBspDraw( (BSPBASE+node)->B)
        add     bx,bx
        mov     bl,es:1[si+bx]   ;get back node
        cmp     bl,255
        je      @F
        push    cx              ;save current node
        mov     cl,bl
        call    NEAR PTR BspSrt
        pop     cx
@@:
        mov     [di],cl         ;*(DRAWPTR++)=node
        inc     di
        mov     bx,cx           ;MakeBspDraw( (BSPBASE+node)->F)
        add     bx,bx
        mov     bl,es:[si+bx]   ;get front node
        cmp     bl,255
        je      @F
        push    cx              ;save current node
        mov     cl,bl
        call    NEAR PTR BspSrt
        pop     cx
@@:
        mov     al,255
        mov     [di],al
        retn
bspback:
        mov     bx,cx           ;MakeBspDraw( (BSPBASE+node)->F)
        add     bx,bx
        mov     bl,es:[si+bx]   ;get front node
        cmp     bl,255
        je      @F
        push    cx              ;save current node
        mov     cl,bl
        call    NEAR PTR BspSrt
        pop     cx
@@:
        mov     bx,dx           ; if( (*(FBPTR+node))&0x80 )
        add     bx,cx
        mov     al,[bx]
        and     al,080h
        je      @F
        mov     [di],cl         ;*(DRAWPTR++)=node
        inc     di
@@:        
        mov     bx,cx           ;MakeBspDraw( (BSPBASE+node)->B)
        add     bx,bx
        mov     bl,es:1[si+bx]   ;get back node
        cmp     bl,255
        je      @F
        push    cx              ;save current node
        mov     cl,bl
        call    NEAR PTR BspSrt
        pop     cx
@@:
        mov     al,255
        mov     [di],al
        retn

MakeBspDrawList PROC   USES es si di,node:WORD

        les     si,BSPBASE
        mov     dx,FBPTR
        mov     di,DRAWPTR
        mov     cx,node
        sub     ch,ch

        call    NEAR PTR BspSrt
        ret

MakeBspDrawList ENDP


.386


BuildPolyList   PROC    uses es si di,Num:WORD,Lst:WORD,NLst:WORD,DST:DWORD,SRC:DWORD,TYP:WORD

PFLGS   = 30

        mov     cx,Num
        or      cx,cx
        jne     @F
        mov     ax,-1           ;bad num of points dont draw
        ret
@@:
        mov     si,Lst          ;get source point ptr array base
        mov     ax,-1
        sub     bx,bx
@@:
        les     di,[si]         ;get pointer
        mov     dx,es:[di].PFLGS        ;get rot flag
        and     ax,dx
        or      bx,dx
        add     si,4
        dec     cx
        jne     @B
        test    ax,ROUTS        ;if all rot out leave
        je      @F
        mov     ax,-1           ;dont draw
        ret
@@:
        test    bx,RZLFLAG      ;any need z clipping ??
        je      @F              
        mov     ax,1            ;dont do it here
        ret
@@:
        mov     si,Lst          ;get source point ptr array base
        mov     di,NLst
        mov     cx,Num
        mov     Oflags,-1
L1:
        les     bx,[si]         ;get pointer
        mov     dx,es:[bx].PFLGS        ;get rot flag
        test    dx,PDFLAG       ;has it been perspd
        jne     @F
        push    cx
        push    es
        push    bx
        call    Perspect3D
        pop     bx
        pop     es
        pop     cx
        mov     dx,es:[bx].PFLGS        ;get rot flag
@@:
        and     Oflags,dx
        mov     [di+2],es
        mov     [di],bx
        add     di,4
        add     si,4
        dec     cx
        jne     L1
        mov     si,Lst          ;get source point ptr array base
        les     bx,[si]         ;get pointer
        mov     [di+2],es
        mov     [di],bx
; draw them here

        mov     ax,Oflags
        test    ax,POUTS
        je      @F
        mov     ax,-1
        ret
@@:
        mov     di,NLst
@@:
        push    TYP
        les     bx,[di+4]
        add     bx,12           ;&(P->XP)
        push    es
        push    bx
        les     bx,[di]
        add     bx,12           ;&(P->XP)
        push    es
        push    bx
        call    NPFLine
        add     sp,10
        add     di,4
        dec     Num
        jne     @B

        mov     eax,SRC
        push    eax
        push    TYP
        mov     eax,DST
        push    eax
        call    NPFFill
        add     sp,10
        mov     ax,0
        ret

BuildPolyList   ENDP


.386

MyWaterline	PROC	USES bp si di bx fs es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR_2:WORD,DST:WORD,GREY:WORD,WID:WORD

        push    bp

	mov	fs,SRCSEG
	mov	di,ADDR_2
	mov	ax,DST
	mov	es,ax

    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	bx,ZADD


	shl	si,1
	shl	bx,1

	mov	ax,WID
	shr	ax,1
	shr	ax,1
	mov	MWID,ax

        mov     ax,GREY
        mov     gs,ax
        or      ax,ax
        je      nogrey
        mov     bp,bx
        mov     ax,MWID
L1:
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
	mov	es:[di],bl      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
	mov	es:1[di],bl      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
	mov	es:2[di],bl      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
	mov	es:3[di],bl      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	add	di,4
	dec	ax
	jne	L1
        pop     bp
	ret
nogrey:
        mov     bp,bx
        shr     MWID,1

L3:
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 14
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	ah,fs:[bx]      ;[4] get base pix
	mov	es:[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 14
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	ah,fs:[bx]      ;[4] get base pix
	mov	es:2[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 14
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	ah,fs:[bx]      ;[4] get base pix
	mov	es:4[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 14
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	ah,fs:[bx]      ;[4] get base pix
	mov	es:6[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

        add	di,8
	dec	MWID
	jne	L3

        pop     bp
        ret
MyWaterline	ENDP
.8086

.386
MyWaterline2	PROC	USES bp si di bx fs es,XS:WORD,ZS:WORD,XADD:WORD,ZADD:WORD,SRCSEG:WORD,ADDR_2:WORD,DST:WORD,GREY:WORD,WID:WORD

        push    bp

	mov	fs,SRCSEG
	mov	di,ADDR_2
	mov	ax,DST
	mov	es,ax

    	mov	cx,XS
	mov	dx,ZS
	mov	si,XADD
	mov	bx,ZADD

        shl     si,1
        shl     bx,1


	shl	si,1
	shl	bx,1

	mov	ax,WID
	shr	ax,1
	shr	ax,1

	mov	MWID,ax

        mov     ax,GREY
        mov     gs,ax
        or      ax,ax
        je      nogrey
        mov     bp,bx
        mov     ax,MWID
        shr     ax,1
L1:
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
        mov     bh,bl
	mov	es:[di],bx      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
        mov     bh,bl
	mov	es:2[di],bx      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
        mov     bh,bl
	mov	es:4[di],bx      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	bl,fs:[bx]      ;[4] get base pix
        sub     bh,bh           ;[2]
        mov     bl,gs:[bx]      ;[4] get greyed pixel value
        mov     bh,bl
	mov	es:6[di],bx      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 22

	add	di,8
	dec	ax
	jne	L1
        pop     bp
	ret
nogrey:
        mov     bp,bx
        shr     MWID,1

L3:
	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
        mov     ah,al
	mov	es:[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
        mov     ah,al
	mov	es:2[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
        mov     ah,al
	mov	es:4[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

	mov	bh,dh           ;[2] decode address
	mov	bl,ch           ;[2]
	and	bx,07f7fh       ;[2] 128 pixel wraparound
	mov	al,fs:[bx]      ;[4] get base pix
        mov     ah,al
	mov	es:6[di],ax      ;[2] save it
	add	cx,si           ;[2] next source address
	add	dx,bp           ;[2]  total clocks = 16  = 30 /2

        add	di,8
	dec	MWID
	jne	L3

        pop     bp
        ret
MyWaterline2	ENDP
.8086



END




