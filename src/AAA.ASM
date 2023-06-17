	DOSSEG
	.MODEL MEDIUM,C
	page 60, 132
_3DOBJS	SEGMENT	PARA PUBLIC 'DAT'

	PUBLIC	AAFIRE
AAFIRE	dw	1

	dw	object0
	dw	std_pts

object0	dw	8001h,AAF

AAF	dw	396

	;num matrices ( if any )

	dw	2
	db	0,1
	dw	0,0,4000

	;num points

	dw	6

	;num surfaces

	dw	0

	;point values

	dw	08000h+1
	dw	0,0,0
	dw	0,57,0
	dw	0,113,0
	dw	0,170,0
	dw	0,226,0
	dw	0,283,0

	;num lines

	dw	3
	; line indicies

	db	0,1
	db	1,2
	db	2,0

	;fills and lines
	db	3 ,0,1,2, -1
	db	6+20h
	db	4Ch,0,1,2,3,4,5

	db	-1
std_pts	dw	0
	END
