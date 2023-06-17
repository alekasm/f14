// Internal functions found in overlay
int (_far *DigSysSetup)(int BoardAddress, int BoardType, int IRQ, int DRQ,
	long BytesLoaded, unsigned EMSsegment, unsigned EMShandle);

/* add phrase to queue */
int (_far *DigQueue)(int phrase);

/* flush the queue */
void (_far *DigFlushQueue)(void);

/* return current playback status */
int (_far *DigStatus)(void);

/* stop anything currently playing      */
int (_far *DigAllOff)(void);

/* stops and DMA in progress and resets driver */
void (_far *DigShutDown)(void);

//***************************************************************************
int dig_load_overlay(char *filename) 
{
	static struct exe_header_data {
		int  codeseg, dataseg, datalen;
		int  dummy[2];
		int  f1; 
	} _far *header;

	char _far *fname;
	static struct {
	int load, rel; 
	} param;
	char _far *code_ptr;
	unsigned int _far *code_offset_ptr;
	unsigned err, i;

	fname=filename;

	_asm {
		mov   bx,-1                     ; Check available mem
		mov   ah, 48H                   ; Alloc mem
		int   21H                       ; Call DOS
//		sub   bx, 400H                  ; avail minus DOS2.1 loader 16Kb overhead
		mov   ah, 48H                   ; Alloc mem again
		int   21H                       ; Call DOS
		mov   param.load, ax            ; Setup parameter block
		mov   param.rel, ax
		mov   ax, SEG param
		mov   es, ax
		mov   bx, OFFSET param          ; ES:[BX] points to parameter block
		lds   dx, fname                 ; DS:[DX] points to overlay name
		mov   ax, 4B03H                 ; Load overlay function
		int   21H                       ; Call DOS
		mov   ax, ss                    ; Move SS ...
		mov   ds, ax                    ; ... back to DS
		mov   ax, 0
		adc   ax, 0
		mov   err, ax                   ; Get error flag from Carry
	}
	if(err) {
		_asm {
			mov   ah, 049h                ; Release memory
			mov   es, param.load
			int   021h                    ; Call DOS
		}
		param.load=0;
		return(1); 
	}
	header=(struct exe_header_data _far *)(((long)param.load<<16)+40);
	i=header->datalen/16+header->dataseg-header->codeseg+1;  // Overlay size

	_asm {
		mov   ah, 4ah                 ; Resize dataseg
		mov   bx, i
		int   021h                    ; Call DOS
	}
	code_ptr=(char _far *)((long)header->codeseg<<16);
	code_offset_ptr=&header->f1;
	// Set function pointers from overlay data
	(char _far *)DigSysSetup  = code_ptr+*code_offset_ptr++;
	(char _far *)DigShutDown  = code_ptr+*code_offset_ptr++;
	(char _far *)DigQueue     = code_ptr+*code_offset_ptr++;
	(char _far *)DigFlushQueue= code_ptr+*code_offset_ptr++;
	(char _far *)DigAllOff    = code_ptr+*code_offset_ptr++;
	(char _far *)DigStatus    = code_ptr+*code_offset_ptr++;
	return(0); 
}
