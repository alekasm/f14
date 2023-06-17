/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    File Name:          MONOPRIN.C

    Author:             Ned Way

    File Date:          22:13:04  Mon  27-Sep-1993

    File Description:
                        printf function for monochrome screen


    Enhancements Needed:



    Routines Implemented:



    Modifications:
        Date        Name            Purpose:



лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/



#include <stdio.h>
#include <stdarg.h>
#include <bios.h>


char _mono_buffer[2000], _mono_attribute = 0x07 ;

int _mono_x = 0, _mono_y = 0 ;

int far *_mono_loc = (int far *) 0xb0000000 ;



/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int mprintf (char *fmt, ...) {


va_list args ;
int length ;


        va_start (args, fmt) ;

#if (_MSC_VER <= 600)
        vsprintf (_mono_buffer, fmt, args) ;
	 #else
        _vsnprintf (_mono_buffer, 2000, fmt, args) ;
#endif

        mono_out (_mono_buffer) ;


}

int mono_scroll (void) {

int far *from = _mono_loc + 80, far *to = _mono_loc ;

int blank = 0x0720 ;

        _fmemcpy (to, from, (2000 - 80) * sizeof (int) ) ;

        _fimemset (_mono_loc + 24 * 80, 80, blank) ;


}

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int mono_out (char *buffer) {

int c, attribute = _mono_attribute << 8 ;

int far *to = _mono_loc ;

        to += _mono_y * 80 + _mono_x ;

        while (*buffer != '\0') {

                switch (c = *buffer++) {



                        case '\r':

                                _mono_x = 0 ;

                                to = _mono_loc + _mono_y * 80 ;


                                break ;

                        case '\n':

                                if (_mono_y == 24)
                                        mono_scroll () ;

                                    else {
                                        to += 80 ;
                                        _mono_y++ ;

                                }


                                break ;

                        case '\f':

                                _mono_x = 0 ;
                                _mono_y = 0 ;


                                to = _mono_loc ;

                                c = attribute | 0x20 ;

                                _fimemset (to, 2000, c) ;    // clears screen

                                break ;

                        case '\t':

                                if (_mono_y == 24 && _mono_x > 71) {
                                        mono_scroll () ;
                                        _mono_y = 23 ;
                                        to = _mono_loc + 23 * 80 ;

                                }

                                c = attribute | 0x20 ;

                                _fimemset (to, 8, c) ;

                                to += 8 ;

                                if (_mono_x > 71) {
                                        _mono_x = (_mono_x + 8) % 80 ;
                                        _mono_y++ ;
                                        }
                                    else
                                        _mono_x += 8 ;


                                break ;

                        default:

                                c |= attribute ;

                                *to++ = c ;

                                _mono_x++ ;


                                if (_mono_x == 80) {
                                        if (_mono_y == 24) {
                                                mono_scroll ;
                                                _mono_x = 0 ;

                                                to = _mono_loc + 24 * 80 ;

                                                }
                                            else {
                                                _mono_x = 0 ;
                                                _mono_y++ ;

                                        }

                                }

                                break ;

                }

        }

        return 0 ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int _fimemset (void far *to, int n, int c) {



        _asm {

                push    es

                mov     cx, n
                mov     ax, c

                les     di, to

                repnz   stosw

                pop     es


        }

        return 0 ;

}


/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


int mset_pos (int x, int y) {


        _mono_x = x % 80 ;
        _mono_y = y % 25 ;

        return 0 ;


}

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/

int mclear_screen (void) {

int blank = (_mono_attribute << 8 ) | 0x20 ;

int far *to = _mono_loc ;

        _fimemset (to, 2000, blank) ;

}

