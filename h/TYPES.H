#ifndef EXEC_TYPES_H
#define EXEC_TYPES_H

#define GLOBAL extern
#define IMPORT extern
#define STATIC static
#define REGISTER register
#define VOID void

typedef long LONG;
typedef signed long SLONG;
typedef unsigned long ULONG;
typedef unsigned long LONGBITS;
typedef signed int SWORD;
typedef unsigned int UWORD;
typedef unsigned short WORDBITS;
typedef char SBYTE;
typedef unsigned char UBYTE;
typedef unsigned char BYTEBITS;
typedef unsigned char *STRPTR;
typedef STRPTR *APTR;
typedef ULONG CPTR;
typedef short SHORT;
typedef unsigned short USHORT;
typedef float FLOAT;
typedef double DOUBLE;
typedef short COUNT;
typedef unsigned short UCOUNT;
typedef short BOOL;
typedef unsigned char TEXT;
typedef int BPTR;
#endif

#ifndef TRUE
#define FALSE	0
#define TRUE	1
#define true	1
#define false	0
#endif

#ifndef NULL
#define NULL	0
#endif
