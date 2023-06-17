

/*
лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

    Routine Name:	save_video_state
			restore_video_state


    Routine Description:
			preserves & restores VGA & above video state


    Enhancements Needed:


    Argument Description:
			void


    Return Value:	void

    Globals Used:       _current_mode, _current_video_mode,
                        _current_video_font, _current_video_rows

    Globals Modified:	None

    Local Variable Descriptions:


    Modifications:
	Date		Name		Purpose


лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл
*/


#define EGA		0
#define VGA		1


int _current_mode = VGA ;

int _current_video_mode = 3, _current_video_font = 0x12 ;
int _current_video_rows = 25 ;

void save_video_state (void) {

int num_blocks, return_val ;


char table[64], far *ptable = table ;


	_asm {

		mov	ah, 1ch
		mov	al, 0
		mov	cx, 111b

		int	10h

		mov	num_blocks, bx

		mov	return_val, ax

	}

	if ( (return_val & 0xff) != 0x1c) {
		_current_mode = EGA ;

		return ;
	}


	_asm {
		mov	ah, 1bh
		mov	bx, 0
		les	di, ptable

		int	10h

	}


	_current_video_mode = table[4] ;


	if ( (_current_video_mode <= 3) || (_current_video_mode == 7) ) {
		if (table[35] == 8) /* 8x8 font */
			_current_video_font = 0x12 ;
		    else if (table[35] == 14) /* 8x14 font */
			_current_video_font = 0x11 ;
		    else if (table[35] == 16) /* 8x16 font */
			_current_video_font = 0x14 ;

		    else {
			_current_video_mode = 3 ;
			_current_video_font = 0x12 ;

		}
		}
	    else {

		_current_video_rows = table[34] ;

		if (table[35] == 8) /* 8x8 font */
			_current_video_font = 0x23 ;
		    else if (table[35] == 14) /* 8x14 font */
			_current_video_font = 0x22 ;
		    else if (table[35] == 16) /* 8x16 font */
			_current_video_font = 0x24 ;

		    else {
			_current_video_mode = 3 ;
			_current_video_font = 0x12 ;

		}


	}



}


void restore_video_state (void) {

int num_blocks, return_val ;
int font, mode, rows ;


        if (_current_mode == EGA)
                return ;


	font = _current_video_font ;
	mode = _current_video_mode ;
	rows = _current_video_rows ;


	_asm {

		mov	ah, 0
		mov	al, byte ptr mode

		int	10h


		mov	ah, 11h
		mov	al, byte ptr font
		mov	bl, 0
		mov	dl, byte ptr rows

		int	10h

	}


}
