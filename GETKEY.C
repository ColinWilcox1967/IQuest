#include <stdio.h>
#include <conio.h>
#include <bios.h>

extern void show_time ( void );
extern int wait_a_second ( void );

unsigned int get_key ( void );

unsigned int get_key ()
{
    union REGS regs;

    regs.h.ah = 0;
    int86(0x16, &regs, &regs);
    
    if (((regs.h.ah == 46) && ( regs.h.al == 3)) ||
        (( regs.h.ah == 0) && ( regs.h.al == 0))
       )
        return (0);

    if ( regs.h.al == 0)
        return ((unsigned int)(128 + regs.h.ah));
    else
        return ( (unsigned int)regs.h.al);
}


