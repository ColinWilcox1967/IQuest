#include <malloc.h>
#include <stdio.h>
#include "palette.h"
#include "keys.h"
#include "iq.h"
#include "config.h"
#include "windows.h"

extern struct palette_str palette;
extern void draw_push_button ( int x, int y, char msg[], int width);

extern int abandon_new_settings ( void );
extern void WriteMultiChar (int x, int y, char ch, int n);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void SetColourPair (short fg, short bk);
extern void simulate_push (int x, int y, char msg[], int width);

void do_border_type ( void );
void show_border_settings ( int option );
void do_border_help ( void );

void do_border_help ()
{
  char *hbuff;

  hbuff = (char *)malloc(PORTSIZE(25, 9, 56, 16));
  if (hbuff != NULL)
  {
     save_area (25, 9, 56, 16, hbuff);
     SetColourPair (palette.black, palette.green);
     draw_window (25, 9, 55, 15);
     window_title (25, 55, 9, "Border Type Help", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     draw_push_button (33, 14,"&Continue");
     restore_area (25, 9, 56, 16, hbuff);
     free ((char *)hbuff);
  }
}

void show_border_setting ( int option )
{
  int i;

  SetColourPair (palette.bright_white, palette.green);
  WriteChar (29, 10, ' ');
  WriteChar (29, 11, ' ');
  WriteChar (29,9 + option,'X');
  SetColourPair (palette.bright_white, palette.blue);
  switch ( option )
  {
     case 1 : 
              WriteChar (40, 9, TL_SINGLE); WriteChar (50, 9, TR_SINGLE);
              WriteChar (40, 12, BL_SINGLE); WriteChar (50, 12, BR_SINGLE);
              WriteMultiChar (41, 9, H_SINGLE, 9);
              WriteMultiChar (41,12, H_SINGLE, 9);
              for (i=10; i<12; i++)
              {
                WriteChar (40, i, V_SINGLE);
                WriteChar (50, i, V_SINGLE);
              }
              break;
     case 2 : 
              WriteChar (40, 9, TL_DOUBLE); WriteChar (50, 9, TR_DOUBLE);
              WriteChar (40, 12, BL_DOUBLE); WriteChar (50, 12, BR_DOUBLE);
              WriteMultiChar (41, 9, H_DOUBLE, 9);
              WriteMultiChar (41,12, H_DOUBLE, 9);
              for (i=10; i<12; i++)
              {
                WriteChar (40, i, V_DOUBLE);
                WriteChar (50, i, V_DOUBLE);
              }

              break;
  }
  SetColourPair (palette.bright_red, palette.green);
  WriteString (43,10, "Sample"); 
  WriteString (43, 11, "Window");
}

void do_border_type ( void )
{
  char         *buff;
  unsigned int key;
  int          option;
  int          finish;
  unsigned char old_config_byte1;

  buff = (char *)malloc(PORTSIZE(20, 8, 61, 16));
  if (buff != NULL)
  {
     save_area (20,8,61,16,buff);
     SetColourPair (palette.black, palette.green);
     draw_window (20,8,60,15);
     window_title (20,60,8,"Window Border Type", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (23, 10, "[   ] Single Line");
     WriteString (23, 11, "[   ] Double Line");
     draw_button (26, 14, "&Ok", 4, BUTTON_UP);
     draw_button (38, 14, "&Cancel", 8, BUTTON_UP);
     old_config_byte1 = config_byte1;
     if (config_byte1 & MSK_BORDER_TYPE)
        option = 2;
     else
        option = 1;
     finish = FALSE;
     while (! finish)
     {
        show_border_setting ( option );
        key = toupper(get_key());
        switch ( key )
        {
           case K_F1      : do_border_help ();
                            break;
           case K_CR      :
           case 'O'       : finish = TRUE;
                            simulate_push (26, 14, "&Ok", 4);
                            break;
           case 'C'       : 
           case K_ESC     : 
                            finish = TRUE;
                            simulate_push ( 38, 14, "&Cancel", 8);
                            if (abandon_new_settings ())
                               config_byte1 = old_config_byte1;
                            break;
           case K_TAB     : 
           case K_CSR_DN  : option++;
                            if (option > 2) option=1;
                            break;
           case K_REVTAB  :
           case K_CSR_UP  : option--;
                            if (option<1)
                                option = 2;
                            break;
           case 'S'       : option=1;
                            break;
           case 'D'       : option=2;
                            break;
           default        : break;
        }
     }
     restore_area (20, 8, 61, 16, buff);
     free((char *)buff);
  }
}


