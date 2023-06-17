## F-14 Fleet Defender Pre-Release
*Source*: https://archive.org/details/f-14-src  

### Build Environment
- DOSBox-X  
- Microsoft Macro Assembler (MASM) 6.11  
- Microsoft C/C++ 7.0  
- HX DOS Extender (DPMI) 2.16

Installing this software should correctly set environment variables to your `AUTOEXEC.BAT`. If you attempt to run without DPMI, you will get an error message:    
```
run-time error R6901
- DOSX32 : This is a protected-mode application that requires DPMI (DOS Protected Mode Interface) services. Examples of hosts that provide these services include:  
 - a DOS session under Windows 3.x in enhanced mode  
 - Qualitas 386MAX or BlueMAX version 6.x  
```  
Initialially I installed Windows 3.x and ran with enhanced mode `WIN /3` but was unsucessful. HXRT216 seems to work just fine, so add it to your `AUTOEXEC.BAT` (or environment script): `HXRT216\BIN\HDPMI32.EXE`.  An issue with running this DPMI is an error message during compilation which states the following:  
```
Phar Lap fatal err 10049: Ran out of stack buffers
```
In my environment script (or `AUTOEXEC.BAT`) you can try adding `set GOTNT=-NOPAGE` prior to executing `HDPMI32.EXE`. This helps a little bit (I think), but it has required me to still reboot and continue attempts to recompile further.

The target we are interested in compiling is `f14.exe` under `src`.

### Changelog
There are various issues with building the source as it stands, some of which are fixed. The first issue was that various C files required headers, and only some C files referenced `INCDIR`. I've simply added `-I$(INCDIR)` to the `CFLAGS` in the `MAKEFILE`. Another issue was that it appeared `PASM.ASM` was using reserved keywords for procedure argument names (namely `ADDR`); I've simply renamed these.   

### Current Issues
The biggest blocker right now is `3dmatrix.asm` fails on the linking stage because it requires definitions for various math `EXTERN`s. I see that the `MAKEFILE` references a `..\lib\mpslibm.lib`, which would presumably have these missing symbol definitions such as `_SinHead`, `_CosPitch`, etc. This static library is not a target in the `MAKEFILE` and does not appear to be included, so this may require people going on a difficult scavenger hunt to find it.
