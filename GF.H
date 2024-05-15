/* GF.H - Supports LARGE model */
#define _LCODE  1
#define _LDATA  1
#define _MSC 5
/*  Required GF.H Header for Microsoft C
*
*  Copyright (C) 1983-88 Greenleaf Software
*
*  To use, copy this file, GF.S to GF.H
*/



#ifndef GFDOTH
#define GFDOTH

#ifndef LINT_ARGS
#define LINT_ARGS
#endif

#define EOS     '\0'
typedef unsigned int    word;
#ifndef GLOS2
typedef char            BYTE;
#endif
typedef int             bool;
#define VIDEO   0x10
#define DOSINT  0x21
struct GFREGS   { int ax,bx,cx,dx,si,di,ds,es; };
#define YES     1
#define NO      0
#define OK      1
#define NOTOK   0
#define SUCCESS 1
#define FAILURE 0
#define ON      1
#define OFF     0
#define SET     1
#define CLEAR   0
#define FOREVER 1
#define NEVER   0
#define ERROR   (-1)
#define NORM    1
#define FAST    2
#define SLOW    3
#define NOBLINK 4
#define INCHES  10
#define LINES   11
#define BLINK   32
#define BLANK   0x20
#define CR      13
#define LF      10
#define FF      12
#define BKSP    8
#define TAB     9
#define ESC     27
#define DEL     0x7F
/* monochrome video attributes */
#define VBLINK 0x80
#define ULINE 1
#define INTENSE 8
#define NORMAL  7
#define VNORMAL 7
#define REVERSE 0x70
#define NONDISP 0
#define WAIT while(1){if(gfkbhit()&&getkey()==ESC)break;else;}
#define iscarry(x) ((x&0x0001)?TRUE:FALSE)

#endif

