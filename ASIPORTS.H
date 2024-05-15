/*
 * asiports.h
 * contains: User include file for the Greenleaf Comm Library
 *
 */

/*
 *
 * MODIFICATIONS
 *  David Nienhiser   03-FEB-1987  11:18:34.27
 *	Added definition for extern int _asoprt.
 *  David Nienhiser   03-FEB-1987  14:34:48.81
 *	txwalert && xoffsent bits in AS_STBITS structure were reversed.
 *	Added constants for COMLIBVERSION && COMLIBREVISION.
 *  David Nienhiser   20-FEB-1987  09:08:15.37
 *	Added structure bits & variables for support of RTS control
 *	 
 *  David Nienhiser   11-MAY-1988  15:35:02.32   Version 2.20
 *	 
 *      Added new structure member called txiflag, this bit will take
 *      the place of the old Transmit interrupt enable bit internal
 *      to the 8250/16450.  At initialization time the transmit interrupt
 *	enable bit will be enabled (if transmit interrupts are enabled).
 *	From this point forward the txiflag bit in the structure will
 *	be what is actually toggled off and on instead of the interrupt
 *	enable bit internal to the 8250/16450.  This was done because
 *	not all serial devices generate an interrupt when the interrupt
 *	enable flag inside the 8250/16450 is asserted.
 *
 *	Added a new function to select an alternate set of interrupt
 * 	scan and lock code.  These new interrupt routine headers are
 *	specifically for multiple port COMM boards that represent channels
 *      needing service by setting bits in the interrupt latch such as the 
 *	Digiboard for the PS/2 and Quadport AT.  The name of the new function
 *	is asihsel() (handler select) and is in the file _asifirs.asm.   
 *	It must be called with a parameter of 1.
 *
 *	Added a new function submodel(), this function returns the
 *	machine submodel byte which is documented on page 2-95 and 4-18
 *	of the IBM BIOS interface technical reference.  This was necessary
 *	to differentiate between PS/2 model 50 & 60 and IBM-AT systems,
 *	the value returned by machine() is the same for both.  submodel()
 *	can be found in machine.asm.
 *
 *	Defined the constant COM17 for the 16 channel digiboard.
 *
 *	Added a new member to the PORTINFO structure, the as_xorv.
 *	This member is xor'ed with the value read from the board's
 *	interrupt identification port.  By default it is 0, if the
 *	board's interrupt identification register identifies channels
 *	that need service with 0's instead of 1's this value should
 *	be set to 0xffff.  An example of where this is needed is with
 *	the Arnet Multiport.
 *	 
 *	Added new function asinvert() to accomplish this.  asinvert()
 *	is in a NEW module, asinvert.c
 *	 
 *	Changed MAX_PORT to 17, for PS/2 & Digiboard 16 channel + COM1
 *
 *	Added POS.... macro's and constants to ease accessing POS registers
 *	on PS/2 w/Micro-Channel.
 *
 *	Version 2.20 of the Greenleaf Comm library has now been tested with
 *	the following:
 *
 *	Digiboard  8 port for IBM PC and AT
 *	Digiboard  4 port for PS/2 Micro Channel
 *	Digiboard 16 port for PS/2 Micro Channel
 *	 
 *	Quadram Quadport-AT  5 channel
 *
 *	Arnet Multiport 8 port for IBM PC and AT
 *
 *	Star Gate Technologies PLUS 8 and PLUS 8 MC
 *
 *	Consult the program MULTLOOP.C for information on how to setup
 *	the Comm library for each of these.
 *
 *	Added new file GLCRC.ASM, which contains function glcrc().  This
 *	function calculates the crc of a variable length buffer.
 *
 *	Modified the XMBUF structure in xmodem.h.
 *	Added member to XMBUF structure named xfermode, this will be set
 *	to CRCMODE if XMODEM-CRC is being used, it will be set to
 *	CHECKSUMMODE if XMODEM-Checksum is being used.  CRCMODE and
 *	CHECKSUMMODE are two new constants added to xmodem.h also.
 *	
 *	Added another parameter to asigetd(), controls whether
 *	character is actually removed from buffer.
 *
 *  Kamy Rahimi		03-6-1988
 *	Changed static int _asidown to int _asidown() to match asiquit.c 
 *	source.
 *
 */

#define	COMLIBVERSION	2		/* Comm library Version */
#define	COMLIBREVISION	21		/* Comm library Revision */


#define MAX_PORT 	17
#define	MAX_BUF_SIZE	65535
#define MIN_BUF_SIZE	2

#define COM1		0
#define COM2		1
#define COM3		2
#define COM4		3
#define COM5		4
#define COM6		5
#define COM7		6
#define COM8		7
#define COM9		8
#define COM10		9
#define COM11		10
#define COM12		11
#define COM13		12
#define COM14		13
#define COM15		14
#define COM16		15
#define	COM17		16

/* Macro's and constants to ease the reading of Micro Channel POS
 * registers.
 */
#define	POSCHNLSELECT	0x96		/* POS Channel Select */
#define	POSLOWIDPORT	0x100		/* POS I.D. Low byte */
#define	POSHIGHIDPORT	0x101		/* POS I.D. High byte */
#define	POS2PORT	0x102		/* POS Adapter info ports 2-5*/
#define	POS3PORT	0x103
#define	POS4PORT	0x104
#define	POS5PORT	0x105
#define	POSCHMIN	0x08		/* Minimum channel */
#define	POSCHMAX	0x0f		/* Maximum channel */
#define	POSCHNLDISABLE	0x00		/* Disable channel access */
#define	POSSELECTCHANNEL(p)	_asoutb(POSCHNLSELECT,p)
#define	POSDESELECTCHANNEL(p)	_asoutb(POSCHNLSELECT,POSCHNLDISABLE)
#define	POSID()		    ((_asinb(POSHIGHIDPORT)<<8)+_asinb(POSLOWIDPORT))
#define	POS2INFO()		_asinb(POS2PORT)
#define	POS3INFO()		_asinb(POS3PORT)
#define	POS4INFO()		_asinb(POS4PORT)
#define	POS5INFO()		_asinb(POS5PORT)


#ifdef	_MSC
#ifndef	_SIZE_T_DEFINED
typedef unsigned int size_t;
#endif
#endif

#ifdef	TURBOC
#ifndef	_SIZE_T
typedef	unsigned size_t;
#endif
#endif

#ifndef	_SIZE_T
#ifndef	_SIZE_T_DEFINED
#ifndef	C86
#ifndef	DATALIGHT
#ifndef	HIGHC
#define	_SIZE_T
#define	_SIZE_T_DEFINED
typedef	unsigned int size_t;
#endif
#endif
#endif
#endif
#endif

#ifdef	TURBOC
 size_t cdecl strlen(const char *s);
 char * cdecl strcpy(char *dest, const char *src);
 char * cdecl strcat(char *dest, const char *src);
 int  cdecl strcmp(const char *string1, const char *string2);
#endif

#ifdef	_MSC
#if	_MSC > 4
 size_t cdecl strlen(const char *);
 char *strcpy(char *,const char *);
 char *strcat(char *,const char *);
 int strcmp(const char *,const char *);
#else
 int strlen(),strcmp();
 char *strcpy(),*strcat();
#endif
#endif


#ifdef	AZTEC
unsigned int strlen(const char *s);
char *strcpy(char *s1, const char *s2);
char *strcat(char *s1, const char *s2);
int strcmp(const char *s1, const char *s2);
#endif


/************************************************************************
*									*
*  USER SHOULD NOT WRITE TO OR ACCESS DIRECTLY ANYTHING PAST THIS POINT *
*									*
************************************************************************/

/* IRQ's 1..8 */
#define IRQ1 1
#define IRQ2 2
#define IRQ3 3
#define IRQ4 4
#define IRQ5 5
#define IRQ6 6
#define IRQ7 7
#define IRQ8 8

/*  port operating modes
*/
#define ASIN 		1
#define ASOUT 		2
#define ASINOUT 	3
#define BINARY		0
#define ASCII	 	4
#define NORMALRX	0
#define WIDETRACKRX	128

/* Error masks for widetrack rx buffer
*/
#define OVERRUN_ERR	  2
#define PARITY_ERR	  4
#define FRAMING_ERR	  8
#define CTS_STATE	 16
#define DSR_STATE	 32
#define RI_STATE	 64
#define CD_STATE	128

/* For asicheck()
*/
#define	IGALERT		1
#define IGCTS		2
#define IGDSR		4
#define IGCD		8
#define	IGMSTAT		16
#define IGLSTAT		32

/* For asirchk()
*/
#define CHKDISABLE 0
#define CHKDISCARD 1
#define CHKFLAGDISCARD 2
#define CHKFLAG 3
#define CHKRESET 4

/*  Define status modes
*/
#define	DIRECT		0
#define STATIC		1

/*  Buffer Flags (Masks)
*/
#define ALERT	1
#define RXEMPTY 2
#define RXFULL	4
#define RXOVFLOW 8
#define TXEMPTY	16
#define TXFULL	32
#define LINERR	64
#define MODCHG	128

/*  Line Status
*/
#define OVERRUN	2
#define PARERR	4
#define FRAMERR	8
#define BREAKDET 16

/*  Modem Status
*/
#define CTSCHG	1
#define DSRCHG	2
#define RITRAIL	4
#define CDCHG	8
#define CTS	16
#define DSR	32
#define RI	64
#define CD	128

/*  ASCII Protocol Characters
*/
#ifndef	NUL
#define NUL 0
#endif
#define SOH 1
#define STX 2
#define ETX 3
#define EOT 4
#define ACK 6
#define DLE 0x10
#define DC1 0x11
#define DC2 0x12
#define DC3 0x13
#define DC4 0x14
#define NAK 0x15
#define ETB 0x17
#define XON 0x11
#define XOFF 0x13
#define NEWLINE '\n'

/* Error codes returned by all functions OR in _aserror.
*/
#define ASSUCCESS		 0
#define ASGENERALERROR		-1
#define	ASINVPORT		-2
#define ASINUSE			-3
#define ASINVBUFSIZE		-4
#define ASNOMEMORY		-5
#define ASNOTSETUP		-6
#define ASINVPAR		-7
#define	ASBUFREMPTY		-8
#define	ASBUFRFULL		-9
#define ASTIMEOUT		-10
#define	ASNOCTS			-11
#define ASNOCD			-12
#define ASNODSR			-13
#define ASNO8250		-14
#define ASXMSTATUS		-15
#define ASUSERABORT		-16
#define ASFILERR		-17
#define	ASXMERROR		-18
#define ASNOWIDERX		-19
#define ASCONFLICT		-20

#define	ASCRCMODE		-21

/*  exit() first does asiquit(-1) then exit()
*/
#define exit(x)	_asiexit(x)

/* Parity types
*/
#define	P_NONE		0
#define P_ODD		1
#define P_EVEN		2
#define P_S_STICK	3
#define P_M_STICK	4

/************************************************************************
*  SUPPORT POLICY RE ALL STRUCTURES FROM HERE ON:			*
*									*
*  These structures are used internally by the Comm functions and the	*
*  interrupts.  Any endeavor to modify them, write to them, or inspect	*
*  values from an applications or other user program will not be	*
*  supported.  We have left the comments in this file intentionally to	*
*  provide some insight into what is going on;  we decline to answer	*
*  any questions on these structures beyond this point.			*
************************************************************************/
/*  Port Status Bits (from interrupt process)
*/
struct AS_STBITS {
	unsigned alert :  1;
	unsigned rxempty: 1;
	unsigned rxfull:  1;
	unsigned rxovflow: 1;
	unsigned txempty: 1;
	unsigned txfull:  1;
	unsigned linerr:  1;
	unsigned modchg:  1;
	unsigned xchrun:  1;
	unsigned rchrun:  1;
	unsigned txwxon:  1;
	unsigned txwcts:  1;
	unsigned txwalert: 1;
	unsigned xoffsent: 1;
	unsigned rtsactive:1;
	unsigned txiflag:1;
	};

/*  Interrupt Options (written from application side, read by interrupts)
*/
struct AS_MODEBITS {
	unsigned is_txint:   1;
	unsigned is_rxint:   1;
	unsigned is_ascii:   1;
	unsigned is_rxerror: 1;
	unsigned is_xoffmode: 1;
	unsigned is_ctsmode: 1;
	unsigned is_igalert: 1;
	unsigned is_igdsr:   1;
	unsigned is_igcd:    1;
	unsigned is_igmstat: 1;
	unsigned is_igrcver: 1;
	unsigned is_xmodemmode: 1;
	unsigned is_polledhdw:  1;
	unsigned is_rchking: 1;
	unsigned is_rtscontrol:1;
	};

/* Port Information Table
*/
struct PORT_TABLE {
	/* 8250 Information
	*/
	int	intrpt_num;	/* 8250 interrupt no. 0C...  */
	unsigned base_8250;	/* base i/o address of 8250  */
	int p_8250[5];		/* previous values for 8250 registers:
				[0] = line control register,
				[1] = modem control register,
				[2] = interrupt enable register,
				[3] = divisor latch least significant byte,
				[4] = divisor latch most significant byte,
				*/
	long p_vector;		/* previous value for interrupt vector */
	unsigned line_stat,	/* line status */
	modem_stat,		/* modem status	*/
	wide_stat;		/* wide-track status */

	/* 8259 Information
	*/
	int irq_8259,		/* Interrupt # in 8259 (com0 = 4) */
	p_8259;			/* Previous value of irq_8259 bit */
	unsigned port_8259; 	/* I/O Address of 8259 */

	/* buffer information
	*/
	unsigned
	    rx_cell,	/* size of transmit cell (for wide-track)*/
	rx_size,	/* size of receive buffer */
	rx_count,	/* number of characters in receive buffer */
	rx_head,	/* offset of receive buffer head */
	rx_tail;	/* offset of receive buffer tail */
	char *rx_buffer; /* always points to base of receive buffer */

	unsigned
	    tx_cell,	/* size of transmit cell (to be like rx-buffer) */
	tx_size,	/* size of transmit buffer */
	tx_count,	/* number of characters in transmit buffer */
	tx_head,	/* offset of transmit buffer head */
	tx_tail;	/* offset of transmit buffer tail */
	char *tx_buffer; /* always points to base of transmit buffer */

	/* port information
	*/
	struct AS_STBITS chst_bits; 	/* port status bits */
	struct AS_MODEBITS chmode_bits;	/* port mode bits IN,OUT,XON,etc */
	unsigned rts_lowater,		/* when to assert RTS */
	rts_hiwater;			/* when to de-assert RTS */
	
	unsigned rx_accum, /* counter for received characters */
	rx_lowater,	  /* low water mark, point at which xon get's sent */
	rx_hiwater;	  /* high water mark, point at which xoff gets sent */
	int stop_xmt,	  /* if xon/xoff enabled character that stops xmit */
	start_xmt,	  /* character that starts xmit */
	stop_rem_xmt,	  /* character sent when rx buffer almost full */
	start_rem_xmt;	  /* character sent when rx buffer almost empty */

	int break_delay;  /* amount of delay for break signal */

	/* time-out values for polled mode
	*/
	int aswmodem,
	aswtime,
	asrtime;

	/* Shared Interrupt Hardware: Status port and bitmask */
	unsigned shioad,
	shbmask;
	unsigned chkchr[3];	/* Each character occupies the low order
				   8 bits, the high order bit (bit 15)
				   is set to 1 if rx-interrupts are to
				   look at this character, bit 14 is set
				   by interrupt routines and is to be
				   checked by asirchk().  Bit 8 & 9 determine
				   the mode or option of what to do */
};


/************************************************************************
*  ARRAYS OF FUNDAMENTAL THINGS THAT MUST BE KNOWN AT INTERRUPT TIME.	*
*									*
*  1.  Pointer to main parameter structure for the port.		*
*  2.  I/O Address of status reg if shared-int hardware, zero if not.	*
*  3.  If shared hardware, this is bitmask to compare for the port.	*
*      Note:  when the int. reads the status port it searches for a 	*
*      match against an entry in this column in the array.		*
*  4.  Value to be XOR'ed with value read from shared port, this will
*      cover the case where bits in the board's interrupt I.D. register
*      are active when 0 instead of active when 1.
************************************************************************/

struct PORTINFO {
	struct PORT_TABLE *ptb;
	unsigned as_shport;
	unsigned as_shbits;
	unsigned as_mask;
	unsigned as_xorv;
};
struct TABLEPORT {
	struct PORTINFO tblport[MAX_PORT];
};

#ifndef MOD_ASIFIRST
extern struct TABLEPORT *as_chnl;
extern unsigned as_8250port[],as_8259ports[];
extern int as_intnums[],as_8259irq[],as_brkdly[],as_wmodem[],
	as_wtime[],as_rtime[],_aserror,_asoprt,_asmask,_asxorv,
	_comvers,_comrev;
extern unsigned as_shioad[],as_shbmask[];
#endif


#define	isalert(p)		_iswhat(p,1)
#define isrxempty(p)		_iswhat(p,2)
#define isrxfull(p)		_iswhat(p,3)
#define isrxovflow(p)		_iswhat(p,4)
#define istxempty(p)		_iswhat(p,5)
#define istxfull(p)		_iswhat(p,6)
#define islinerr(p)		_iswhat(p,7)
#define ismodemerr(p)		_iswhat(p,8)
#define istxintrunning(p)	_iswhat(p,9)
#define isrxintrunning(p)	_iswhat(p,10)
#define isigalert(p)		_iswhat(p,11)
#define isigcts(p)		_iswhat(p,12)
#define isigdsr(p)		_iswhat(p,13)
#define isigcd(p)		_iswhat(p,14)
#define isigmstat(p)		_iswhat(p,15)
#define isiglstat(p)		_iswhat(p,16)

#define isoverrun(p,o)		_isstat(p,o,1)
#define isparityerr(p,o)	_isstat(p,o,2)
#define isframerr(p,o)		_isstat(p,o,3)
#define isbreak(p,o)		_isstat(p,o,4)
#define iscts(p,o)		_isstat(p,o,5)
#define isdsr(p,o)		_isstat(p,o,6)
#define iscd(p,o)		_isstat(p,o,7)
#define isri(p,o)		_isstat(p,o,8)
#define ischgcts(p,o)		_isstat(p,o,9)
#define ischgdsr(p,o)		_isstat(p,o,10)
#define ischgcd(p,o)		_isstat(p,o,11)
#define ischgri(p,o)		_isstat(p,o,12)

#define isncts(p)		_iswhat(p,17)
#define isndsr(p)		_iswhat(p,18)
#define isncd(p)		_iswhat(p,19)
#define isring(p)		_iswhat(p,20)
#define isxmrxing(p)		_iswhat(p,21)
#define isxoffblocked(p)	_iswhat(p,22)
#define isctsblocked(p)		_iswhat(p,23)


#ifdef	LINT_ARGS
unsigned int glcrc(unsigned int,unsigned int,char *);
void _assti(void);
void _ascli(void);
int _asgetc(unsigned);
int _asputc(unsigned,int);
int _asinb(unsigned);
int _asoutb(unsigned,int);
int _asigetc(struct PORT_TABLE *);
int _asiputc(struct PORT_TABLE *,int);
int _asipekc(struct PORT_TABLE *);
void _asregister(struct TABLEPORT *);
int _asifirst(int,int,struct PORT_TABLE *);
void _asiprime(struct PORT_TABLE *);
int interror(void);
int asitime(void);
int _asrts(unsigned,int,struct PORT_TABLE *);
int _asdtr(unsigned,int);
void _asireset(unsigned,int,int,int,int);
void _asiresume(unsigned,int,struct PORT_TABLE *);
void _asihold(unsigned,int,struct PORT_TABLE *);
int _asiinit(unsigned,unsigned,unsigned);
void _asibreak(unsigned,int);
int _asistart(struct PORT_TABLE *);
void _asiquit(int,struct PORT_TABLE *);
void _asidiag(unsigned,int);
int _asgetbaud(unsigned);
unsigned machine(void);
void timer(unsigned);
int asiflow(int,int,int,int,int);
struct PORT_TABLE *_aschkcnl(int);
int asdtr(int,int);
int asgetc(int);
int asibreak(int,int);
unsigned int asibstat(int);
int asiclear(int,int);
int asidiag(int,int);
unsigned int asierst(int);
int asifirst(int,unsigned int,unsigned int,unsigned int);
int asilibrary(int);
unsigned int asigetb(int,char *,unsigned int);
int asigetc(int);
int asigetparms(int,int *,int *,int *,int *,int *,int *);
unsigned int asigets(int,char *,unsigned int,int);
int asihold(int,unsigned int);
int asicheck(int,int,int);
int asiignore(int,int,int);
int asiinit(int,int,int,int,int);
unsigned int asilrst(int);
unsigned int asilstat(int,int);
int hmreset(int);
int hmregread(int,int);
void icat(char *,int);
int hmregset(int,int,int);
int hmringnum(int,int);
int hmringcnt(int);
int hmescset(int,char);
int hms3set(int,char);
int hms4set(int,char);
int hms5set(int,char);
int hmdialset(int,int);
int hmcarrset(int,int);
int hmcomaset(int,int);
int hmcarrec(int,int);
int hmcarrdisc(int,int);
int hmdialrate(int,int);
int hmescguard(int,int);
int hmuart(int);
int hmoption(int);
int hmflags(int);
int hmonline(int);
int hmdialmode(int,int);
int hmdial(int,char *);
int hmrepeat(int);
int hmreverse(int,char *);
int hmanswer(int);
int _hm3(int,char *,int);
int hmcarron(int,int);
int hmecho(int,int);
int hmduplex(int,int);
int hmhook(int,int);
int hmspeak(int,int);
int hmquiet(int,int);
int hmextend(int,int);
int hmverbose(int,int);
int hmstr(int,char *);
unsigned int asimrst(int);
unsigned int asimstat(int,int);
int asiopen(int,unsigned int,unsigned int,unsigned int,int,int,int,int,int,int);
int asipeek(int);
unsigned int asiputb(int,char *,unsigned int);
int asiputc(int,int);
unsigned int asiputs(int,char *,int);
int asiquit(int);
int _asidown(int);
void _asiexit(int);
int asirchk(int,int,unsigned int,int);
int asireset(int);
int asiresume(int,unsigned int);
int asisetv(int,unsigned int,int,unsigned int,int,int,int,int,int,unsigned int,unsigned int);
int asishare(int);
int asistart(int,unsigned int);
unsigned int asiwgetb(int,char *,unsigned int,char *);
int asiwgetc(int,char *);
unsigned int asiwgets(int,char *,unsigned int,int,char *);
int asiwpeek(int,char *);
int asixack(int,int);
int asixnak(int,int,int,int);
int asixoff(int);
int asixon(int,int,int,int,int);
int asixpreface(int,long);
unsigned int asixrst(int);
int asputc(int,int);
unsigned int asputs(int,char *,int);
int asrts(int,int);
int digibord(int,unsigned int,unsigned int,unsigned int,int,int,int,int,int,int,int,int,int,int);
unsigned int getrxcnt(int);
unsigned int gettxfree(int);
int isrchk(int,int);
int asigetd(int,int,int);
int asiputd(int,char,int);
int _isstat(int,int,int);
int isxmrxcnt(int,unsigned int);
int _iswhat(int,int);
int _rts(unsigned,int,struct PORT_TABLE *);
void *malloc(size_t);
void *calloc(size_t,size_t);
void free(void *);
void asihsel(int);
int  submodel(void);
int asinvert(unsigned);
#else
void _asiexit();
extern struct PORT_TABLE *_aschkcnl();
extern unsigned int glcrc();
extern unsigned machine();
extern unsigned int asibstat();
extern unsigned int asibstat();
extern unsigned int asierst();
extern unsigned int asigetb();
extern unsigned int asigets();
extern unsigned int asilrst();
extern unsigned int asilstat();
extern unsigned int asimrst();
extern unsigned int asimstat();
extern unsigned int asiputb();
extern unsigned int asiputs();
extern unsigned int asiwgetb();
extern unsigned int asiwgets();
extern unsigned int asixrst();
extern unsigned int asputs();
extern unsigned int getrxcnt();
extern unsigned int gettxfree();
char *malloc();
char *calloc();
#ifndef	LATTICE
void free();
#endif
#endif


