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
Initialially I installed Windows 3.x and ran with enhanced mode `WIN /3` but was unsucessful. HXRT216 seems to work just fine, so add it to your `AUTOEXEC.BAT` (or environment script): `HXRT216\BIN\HDPMI32.EXE`.  

The target we are interested in compiling is `f14.exe` under `src`.
