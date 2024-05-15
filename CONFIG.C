/* Rewrite 'cos some stupid bastard overwrote the original source */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "keys.h"
#include "config.h"
#include "iq.h"
#include "palette.h"
#include "windows.h"

int save_new_config ( void );
void configure ( void );

extern struct palette_str palette;
extern unsigned char far *video_base;
extern int file_exists (char []);
extern int get_yesno (int x, int y, short fg, short bk);

unsigned char far *old_base;

unsigned char config_byte1,
              config_byte2,
              initial_config_byte1,
              config_byte2,
              initial_config_byte2,
              config_byte3,
              initial_config_byte3;
unsigned int  changed;

int save_new_config ()
{
    char *save_config_buff;
    int answer;
    unsigned char config1_saved,
                  config2_saved,
                  config3_saved;


    /* Only give prompt if there are actually any changes */

    if (( config_byte1 == initial_config_byte1) &&
        ( config_byte2 == initial_config_byte2) &&
        ( config_byte3 == initial_config_byte3)
       )
       return (FALSE);

    save_config_buff = (char *)malloc(PORTSIZE(22, 10, 59, 14));
    if ( save_config_buff == NULL)
        return (FALSE);     
    
    /* When displaying the "Save Configuration" Windows,
       use the initial config byte3 for getting the border type
    */

    config1_saved = config_byte1;
    config2_saved = config_byte2;
    config3_saved = config_byte3;
    config_byte1 = initial_config_byte1;
    config_byte2 = initial_config_byte2;
    config_byte3 = initial_config_byte3;

    save_area ( 22, 10, 59, 14, save_config_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window ( 22, 10, 58, 13);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 29, 11, "Save New Configuration ?");
    answer = get_yesno (33, 12, palette.yellow, palette.red);
    restore_area ( 22, 10, 59, 14, save_config_buff);
    free ((char *)save_config_buff);
    
    /* Put it back as it was */

    config_byte1 = config1_saved;
    config_byte2 = config2_saved;
    config_byte3 = config3_saved;
    return ( answer);
}

void configure ()
{
}


