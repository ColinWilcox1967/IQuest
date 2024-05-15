#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <malloc.h>
#include "iq.h"
#include "windows.h"
#include "keys.h"
#include "palette.h"

extern void write_blanks (int x, int y, int n);
extern void print_file ( char filename[]);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern struct palette_str palette;
extern unsigned int get_key ( void );
extern void draw_button (int x1, int y1, int n, char msg[], int state);
extern void simulate_push (int x1, int y1, char msg[], int n);
extern void GetFileFromDirectory ( void);

extern char current_file[];
extern char gen_filename[];

int select_file_to_print ( void );
void show_print_options ( void );

void show_print_options ()
{
    char         output[40];
    char         drive[_MAX_DRIVE];
    char         path[_MAX_PATH];
    char         name[_MAX_FNAME];
    char         ext[_MAX_EXT];


    SetColourPair (palette.yellow, palette.blue);
    WriteString (23,  10, "Select New File");

    if (current_file[0] == (char)0)
       SetColourPair (palette.white, palette.blue);
    WriteString (23, 12, "Print Current File");
    SetColourPair (palette.red, palette.blue);
    WriteChar (23, 10, 'S'); 
    if (current_file[0] != (char)0)
        WriteChar (23, 12, 'P');
    SetColourPair (palette.cyan, palette.blue);
    write_blanks (23, 13, 36);
    if (current_file[0] != (char)0)
    {
        _splitpath (current_file, drive, path, name, ext);
        sprintf (output, "%s%s%s", path, name, ext);
        if (strlen(output) > 35)
           sprintf (output, "..\\%s%s", name, ext);
    }
    else
       strcpy (output, "NONE");
    WriteString (23, 13, output);
}

int select_file_to_print ( void )
{
   char         *buff;
   unsigned int key;
   int          finish;
   int          abort;

   buff = (char *)malloc(PORTSIZE(20, 8, 61, 17));
   if (buff != NULL)
   {
     save_area (20, 8, 61, 17, buff);
     SetColourPair (palette.bright_white, palette.blue);
     draw_window (20, 8, 60, 16);
     window_title (20, 60, 8, "Print Selection", palette.red, palette.blue);
     show_print_options();
     draw_button (37, 15, 8, "&Cancel", BUTTON_UP);
     finish = FALSE;
     abort = FALSE;
     while (! finish)
     {
        key = get_key();
        switch ( key )
        {
           case 'P'   :
           case 'p'   : if (current_file[0] != (char)0)
                           finish = TRUE;
                        break;
           case 'S'   :
           case 's'   : 
                        GetFileFromDirectory ();
                        strcpy (current_file, gen_filename);
                        show_print_options();
                        break;
           case K_ESC :
           case 'C'   :
           case 'c'   : finish = TRUE;
                        abort = TRUE;
                        simulate_push (37, 15, "&Cancel", 8);
                        break;
           default    : break;
        }
     }
     restore_area (20, 8, 61, 17, buff);
     free ((char *)buff);
   }
   return (abort);
}
