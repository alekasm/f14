;----------;
;
;  File:  TrgTbl.Asm
;
;  Auth:  Andy Hollis  2/12/87
;
;  Edit:  AWH 3/13/92
;
;  This file contains the trig table for Gunship.
;
;  Copyright (c) 1987 by MicroProse Software, All Rights Reserved
;
;----------;

        .MODEL  MEDIUM
;
        PUBLIC  _Isin, _Icos, _TrgMul, _DpTrgMul, _Mul256Div
        PUBLIC  F_SinDP, F_CosDP

        .CODE

_TrgMul PROC

        mov     bx,sp

        mov     ax,SS:[bx+4]
        imul    WORD PTR SS:[bx+6]
        shl     ax,1
        rcl     dx,1
        shl     ax,1    ; round it
        adc     dx,0
        mov     ax,dx

        ret

_TrgMul ENDP

_DpTrgMul PROC

        mov     bx,sp

        mov     ax,SS:[bx+4]
        imul    WORD PTR SS:[bx+6]
        shl     ax,1
        rcl     dx,1

        ret

_DpTrgMul ENDP

_Mul256Div PROC
        mov     bx,sp

        mov     ax,SS:[bx+4]
        cwd
        mov     dl,ah
        mov     ah,al
        sub     al,al
        mov     bx,SS:[bx+6]
        or      bx,bx
        jle     TooBig
        mov     cx,dx
        or      cx,cx
        jns     Pos
        neg     cx
Pos:    shr     bx,1
        cmp     cx,bx
        jae     TooBig
        rcl     bx,1
        idiv    bx

        ret

TooBig: mov     ax,32767
        or      dx,dx
        jns     Pos1
        neg     ax
Pos1:   ret

_Mul256Div ENDP


_Icos    PROC

        mov     bx,sp

        mov     bx,SS:[bx+4]
        call    CosDP
        mov     ax,bx

        ret

_Icos    ENDP


_Isin    PROC

        mov     bx,sp

        mov     bx,SS:[bx+4]
        call    SinDP
        mov     ax,bx

        ret

_Isin    ENDP



F_SinDP PROC    FAR                     ; Far callable version of SinDP

        call    SinDP
        ret

F_SinDP ENDP



F_CosDP PROC    FAR                     ; Far callable version of CosDP

        call    CosDP
        ret

F_CosDP ENDP

;----------------------------------------------------------------------------;
;  SinDP - Calculate a 16-bit signed fraction for Sin (x) / 2 from a 16-bit
;          unsigned angle. Uses interpolation.
;
;     inputs:   BX - 16-bit angle in byte degrees
;
;     outputs:  BX - Sin (x) / 2
;
;     regs:     AX & DX are destroyed.
;
;----------------------------------------------------------------------------;
CosDP:  add     bx,4000h
SinDP:  mov     dl,bl                   ; put low 8 bits in dx
        sub     dh,dh
        mov     bl,bh                   ; make hi 8 bits a table index
        mov     bh,dh
        shl     bx,1

        mov     ax,TrgTbl+2 [bx]        ; get next table value
        mov     bx,TrgTbl [bx]          ; get this table value
        sub     ax,bx                   ; find delta
        imul    dx                      ; multiply by low 8 bits of angle
        mov     dh,dl
        mov     dl,ah
        shl     al,1                    ; round it.
        adc     bx,dx                   ; add it to value for hi 8 bits
        ret


        .DATA

;----------------------------------------------------------------------------;
;  TrgTbl - contains signed 16-bit values approximating Sin(x).
;
;       Example:
;
;           Find Y * Sin (X), where Y is in AX, X (byte degrees) in BX.
;
;           shl   bx,1                  ; make a word index
;           mov   dx,TrgTbl [bx]        ; move Sin value to DX
;           imul  dx                    ; do signed multiply
;           shl   ax,1                  ; final multiply by 2
;           rcl   dx,1                  ; answer is in DX.
;
;----------------------------------------------------------------------------;

;       
;
       PUBLIC  _TRGTB

_TRGTB LABEL WORD

TrgTbl  DW 0,804,1608,2411,3212,4011,4808,5602
        DW 6393,7180,7962,8740,9512,10279,11039,11793
        DW 12540,13279,14010,14733,15447,16151,16846,17531
        DW 18205,18868,19520,20160,20788,21403,22006,22595
        DW 23170,23732,24279,24812,25330,25833,26320,26791
        DW 27246,27684,28106,28511,28899,29269,29622,29957
        DW 30274,30572,30853,31114,31357,31581,31786,31972
        DW 32138,32286,32413,32522,32610,32679,32729,32758
        DW 32767,32758,32729,32679,32610,32522,32413,32286
        DW 32138,31972,31786,31581,31357,31114,30853,30572
        DW 30274,29957,29622,29269,28899,28511,28106,27684
        DW 27246,26791,26320,25833,25330,24812,24279,23732
        DW 23170,22595,22006,21403,20788,20160,19520,18868
        DW 18205,17531,16846,16151,15447,14733,14010,13279
        DW 12540,11793,11039,10279,9512,8740,7962,7180
        DW 6393,5602,4808,4011,3212,2411,1608,804
        DW 0,-804,-1608,-2411,-3212,-4011,-4808,-5602
        DW -6393,-7180,-7962,-8740,-9512,-10279,-11039,-11793
        DW -12540,-13279,-14010,-14733,-15447,-16151,-16846,-17531
        DW -18205,-18868,-19520,-20160,-20788,-21403,-22006,-22595
        DW -23170,-23732,-24279,-24812,-25330,-25833,-26320,-26791
        DW -27246,-27684,-28106,-28511,-28899,-29269,-29622,-29957
        DW -30274,-30572,-30853,-31114,-31357,-31581,-31786,-31972
        DW -32138,-32286,-32413,-32522,-32610,-32679,-32729,-32758
        DW -32767,-32758,-32729,-32679,-32610,-32522,-32413,-32286
        DW -32138,-31972,-31786,-31581,-31357,-31114,-30853,-30572
        DW -30274,-29957,-29622,-29269,-28899,-28511,-28106,-27684
        DW -27246,-26791,-26320,-25833,-25330,-24812,-24279,-23732
        DW -23170,-22595,-22006,-21403,-20788,-20160,-19520,-18868
        DW -18205,-17531,-16846,-16151,-15447,-14733,-14010,-13279
        DW -12540,-11793,-11039,-10279,-9512,-8740,-7962,-7180
        DW -6393,-5602,-4808,-4011,-3212,-2411,-1608,-804
        DW 0


END


