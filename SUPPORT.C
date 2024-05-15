#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bios.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <graph.h>
#include <string.h>
#include "palette.h"
#include "config.h"
#include "keys.h"
#include "windows.h"
#include "iq.h"

extern unsigned char config_byte1;

extern struct palette_str palette;
extern unsigned char far  *video_base;
extern short              current_foreground;
extern short              current_background;
extern unsigned char      initial_config_byte1;
extern struct dostime_t   last_update_time;

extern unsigned int get_key ( void );

void scroll_up_area (int x1, int y1, int x2, int y2);
void scroll_down_area (int x1, int y1, int x2, int y2);
void flush_keyboard ( void );
void centre_text ( int x1, int x2, int y, char msg[]);
void centre_push_button ( int x1, int x2, int y, char msg[]);
int  file_exists ( char name[]);
int  difference ( int a, int b);
void write_blanks ( int x, int y, int n);
void push_wait ( void );
int get_yesno ( int x, int y, short fg, short bk);
unsigned int draw_button (int x, int y, int width, char msg[], int state);
void window_title ( int x1, int x2, int y, char msg[], short fg, short bk);
void simulate_push ( int x, int y, char msg[], int width);
void file_not_found ( void );
void get_name_stub ( char name[]);
int  same_char (char ch1, char ch2);
int overwrite_file ( void );
int  wait_a_second ( void );

union REGS regs;

char n_str[255];
char exp_name[255];

void scroll_up_area (int x1, int y1, int x2, int y2)
{
   int x, y;
   unsigned char b, a;

   for (y=y1; y < y2; y++)
     for (x=x1; x <= x2; x++)
     {
        b = *(video_base+CHAR_POS(x,y+1));
        a = *(video_base+ATTR_POS(x,y+1));
        *(video_base+CHAR_POS(x,y)) = b;
        *(video_base+ATTR_POS(x,y)) = a;
     }
   for (x=x1; x <= x2;x++)
     *(video_base+CHAR_POS(x,y2)) = ' ';
}

void scroll_down_area (int x1, int y1, int x2, int y2)
{
   int x, y;
   unsigned char b,a;

   for (y=y2; y >= y1+1; y--)
     for (x=x1; x<=x2;x++)
     {
        b = *(video_base+CHAR_POS(x,y-1));
        a = *(video_base+ATTR_POS(x,y-1));
        *(video_base+CHAR_POS(x,y))=b;
        *(video_base+ATTR_POS(x,y))=a;
     }
     for(x=x1; x<=x2; x++)
        *(video_base+CHAR_POS(x,y1)) = ' ';
}

int overwrite_file ()
{
    char         *exists_buff;
    int          retval;

    exists_buff = (char *)malloc(PORTSIZE (22, 10, 59, 14)); 
    if ( exists_buff == NULL)
        return(0);
    save_area ( 22, 10, 59, 14, exists_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (22, 10, 58, 13);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 24, 11, "File Already Exists - Overwrite ?");
    retval = get_yesno (33,12, palette.yellow, palette.red);
    restore_area (22, 10, 59, 14, exists_buff);
    free ((char *)exists_buff);
    return ( retval );
}

int same_char (char ch1, char ch2)
{
    if (isalpha (ch1) && isalpha (ch2))
    {
        return ( (ch1 == ch2) || ( ch1 == ch2 + 32) || (ch1 == ch2 - 32));
    }
    else
        return ( ch1 == ch2);
}

void get_name_stub (char *filename)
{
    int idx;

    memset (exp_name, 0, sizeof (exp_name));
    idx = 0;
    while (( filename[idx] != '.') && ( idx < 9 ))
        exp_name[idx++] = filename[idx];
    if ( filename[idx] == '.')
       idx++;
    exp_name[idx] = (char)0;

}

int get_yesno (int x, int y, short foreground, short background)
{
    int          option,
                 got_yn;
    unsigned int key;

    got_yn = FALSE;
    option = 0;
    while (! got_yn)
    {
        
        SetColourPair (foreground, background);
        WriteString (x, y, "(Y)es or (N)o");
        if ((initial_config_byte1 & MSK_VIDEO_DETECT) == MSK_VIDEO_MONO)
            SetColourPair (palette.bright_white, palette.black);
        else
            SetColourPair (palette.dark_grey, palette.bright_white);
        if (option == 0)
            WriteString (x, y, "(Y)es");
        else
            WriteString (x+9, y, "(N)o");
        key = get_key();
        switch ( key )
        {
            case K_CSR_LT    : 
                               option = 0;
                               break;
            case K_CSR_RT    : 
                               option = 1;
                               break;
            case 'Y'         : 
            case 'y'         : option = 0;
                               got_yn = TRUE;
                               break;
            case 'N'         : 
            case 'n'         : option = 1;
                               got_yn = TRUE;
                               break;
            case K_CR        : 
                               got_yn = TRUE;
                               break;
            default          : break;
        }
    }
    return (( key == 'Y') || (key == 'y') || ((key == K_CR) && ( option == 0)));
}

int file_exists (char *name)
{
   return ( access ( name, 0) == 0);
}
    
void file_not_found ()
{
   char *buff;

   buff = (char *)malloc(PORTSIZE(25, 8, 56, 14));
   if (buff != NULL)
   {
     save_area (25, 8, 56,14, buff);
     SetColourPair (palette.bright_white, palette.red);
     draw_window (25, 8, 55, 13);
     SetColourPair (palette.yellow, palette.red);
     centre_text (25, 55, 10, "File Not Found");
     centre_push_button (25, 55, 12, "&Continue");
     restore_area (25, 8, 56, 14, buff);
     free ((char *)buff);
   }
}

void write_blanks (int x, int y, int n)
{
    int idx;

    for (idx = 0; idx < n; idx++)
       WriteChar (x+idx, y, ' ');
}

int difference (int ch1, int ch2 )
{
    int v;

    v = ch1-ch2;
    return ( v > 0 ? v : -v );
}

void hide_cursor ()
{
    regs.h.ah = BIOS_SET_CURSOR_TYPE;
    regs.h.ch = 17;
    regs.h.cl = 16;
    int86 ( BIOS_VIDEO, &regs, &regs );
}

void show_cursor ()
{
    regs.h.ah = BIOS_SET_CURSOR_TYPE;
    regs.h.ch = 1;
    regs.h.cl = 15;
    int86 ( BIOS_VIDEO, &regs, &regs );
}

char *string ( char ch, int n )
{
    int idx;

    for ( idx = 0; idx < n; idx++)
       n_str[idx] = ch;
    n_str[n] = (char)0;
    return (n_str);
}

void SetColourPair ( short foreground, short background )
{
    current_foreground = foreground;
    current_background = background;

    _settextcolor ( foreground );
    _setbkcolor ( (long)background );

}

void ShowHeader ( int x1, int x2, int y, char *header)
{
    int pos = ( x2 - x1 + 1 - strlen (header))/2;

    SetColourPair ( palette.red, palette.green);
    WriteString ( x1 + pos, y, header);
    SetColourPair (palette.black, palette.green);
}

void WriteChar (int x, int y, char ch )
{
    char str[2];

    if ( config_byte1 & MSK_MEMORY_WRITE)
    {
       *(video_base+CHAR_POS(x,y)) = ch;
       *(video_base+ATTR_POS(x,y)) = (char)(ATTR_BYTE(current_foreground, current_background));
    }
    else
    {
        str[0] = ch; str[1] = (char)0;
        _settextposition ( y, x);
        _outtext ( str );
    }
}

void WriteMultiChar (int x, int y, char ch, int n)
{
    int i;

    for (i=0; i < n; i++)
       WriteChar (x+i, y, ch);
}

void WriteString ( int x, int y, char *str )
{
    int i;

    for (i = 0; i < strlen (str); i++)
        WriteChar ( x + i, y, str[i]);
}

void push_wait ()
{
   struct dostime_t start,
                    finish;
   int              time_up;

   _dos_gettime (&start);
   time_up = FALSE;
   while (! time_up)
   {
     _dos_gettime (&finish);
     time_up = (((100 * finish.second)+finish.hsecond) - 
                ((100 * start.second)+start.hsecond) >= 75);
   }
}

int wait_a_second ()
{
   struct dostime_t start;

   _dos_gettime (&start);
   return (((100 * start.second) + start.hsecond) - 
           ((100 * last_update_time.second) + last_update_time.hsecond) >= 100
          );
}


int button_msg_length (char msg[])
{
  int i,
      len;

  len = 0;
  for (i = 0; i < strlen(msg); i++)
     if (msg[i] != '&')
       len++;
  return(len);
}

unsigned int button_msg (int x, int y, char str[], int state)
{
   int i;
   int idx;
   unsigned int key;

   idx = 0;
   i = 0; 
   key = K_CR;
   while (idx < strlen (str))
   {
      if (str[idx] == '&')
      {
         idx++;
         key = (unsigned int)(str[idx]);
         if (state == BUTTON_UP)
           SetColourPair (palette.red, palette.white);
         else
           SetColourPair (palette.white, palette.black);
      }
      else
      {
         if (state == BUTTON_UP)
           SetColourPair (palette.black, palette.white);
         else
           SetColourPair (palette.white, palette.black);
      }
      WriteChar (x+i, y, str[idx]);
      idx++;
      i++;
   }
   return (key);
}

unsigned int draw_button ( int x, int y, int width, char msg[], int state)
{
   if (state == BUTTON_UP)
      SetColourPair (palette.black, palette.bright_white);
   else
      SetColourPair (palette.bright_white, palette.black);
   write_blanks (x, y, width);
   return(button_msg (x + 1, y, msg, state));
}

void simulate_push (int x, int y, char msg[], int width)
{
   unsigned int key;

   key = draw_button ( x, y, width, msg, BUTTON_DOWN);
   push_wait();
   key = draw_button ( x, y, width, msg, BUTTON_UP);
}

void push_button (int x, int y, char msg[])
{
    int width;
    unsigned int  key,
                  active_key;
    int got_key;

    width = button_msg_length (msg) + 2;
    active_key = draw_button (x, y, width, msg, BUTTON_UP);
    got_key = FALSE;
    while (! got_key)
    {
       key = get_key();
       got_key = ((same_char ((char)key, (char)active_key)) || (key == K_CR));
    }
    draw_button(x,y, width, msg, BUTTON_DOWN);
    push_wait();
}

void centre_text ( int x1, int x2, int y, char msg[])
{
  int width;

  width = x2-x1;
  WriteString (x1 + (( width - strlen( msg )) / 2), y, msg);
}

void centre_push_button ( int x1, int x2, int y, char msg[])
{
   int width; 
   
   width = x2 - x1;
   push_button (x1 + ((width - button_msg_length(msg))/2), y, msg);
}

void window_title ( int x1, int x2, int y, char msg[], short fg, short bk)
{
   char str[80];

   sprintf (str, " %s ", msg);
   SetColourPair (fg, bk);
   centre_text ( x1+1, x2, y, str);
}

void flush_keyboard()
{
    while (kbhit ())
        get_key();
}


