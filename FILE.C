#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <malloc.h>
#include "iq.h"
#include "config.h"
#include "keys.h"
#include "windows.h"
#include "palette.h"

#define MAX_PATH 47

extern unsigned int getstr (int x, int y, char str[], char special[], int n);
extern void write_blanks ( int x, int y, int n);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void file_not_found ( void );
extern void TextViewer ( char filename[]);
extern int file_exists ( char name[]);
extern void DrawFileViewer ( char name[]);
extern void UseViewer ( char name[], long size );
extern void GetFileFromDirectory ( void );

extern int file_list_count;
extern char file_list[MAX_FILE_LIST][80];
extern FILE *vwr_file;
extern unsigned char config_byte1;

extern char hotkey[];
extern struct palette_str palette;
extern char gen_filename[];
extern char current_file[];

void do_print_file ( void );
void do_view_file ( void );

/*************/
/* View File */
/*************/

void do_view_file ()
{
   int          idx;
   char         *buff;
   char         *vwr_buff;
   unsigned int key;
   char         name[MAX_PATH+1];
   char         hotkey[3];
   struct find_t c_file;

   buff = (char *)malloc(PORTSIZE(15,10,66,16));
   if (buff != NULL)
   {
      save_area (15,10,66,16,buff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (15,10,65,15);
      SetColourPair (palette.yellow, palette.blue);
      WriteString (25, 11, "Enter Location Of File To View");
      SetColourPair (palette.bright_white, palette.black);
      write_blanks ( 17, 13, MAX_PATH);
      window_title ( 15, 65, 15, "ALT-L For File List", palette.red, palette.blue);
      hotkey[0] = K_ALT_L;
      hotkey[1] = K_ESC;
      hotkey[2] = (char)0;
      SetColourPair (palette.bright_white, palette.black);
      key = getstr (17, 13, name, hotkey, MAX_PATH);
      if (key != 0)
      {
         switch ( key)
         {
            case K_ALT_L : 
                           GetFileFromDirectory();
                           strcpy (name, gen_filename);
                           break;
            case K_ESC   : name[0] = (char)0;
                           break;
         }
      }
      
      if (name[0] != (char)0)
      {
          if (file_exists (name))
          {
              strcpy (current_file, name);

              // updated previously viewed file list too !

              if (file_list_count == MAX_FILE_LIST)
              {
                 for (idx = 0; idx < MAX_FILE_LIST; idx++)   
                    strcpy(file_list[idx], file_list[idx+1]);   
                 strcpy(file_list[MAX_FILE_LIST-1], name);
              }
              else
                 strcpy (file_list[file_list_count++], name);

              if (config_byte1 & MSK_FILE_VIEWER) /* Hex Viewer */
              {
                  _dos_findfirst (name, _A_NORMAL, &c_file);
                  vwr_buff = (char *)malloc (PORTSIZE(1,1,80,23));
                  if (vwr_buff != NULL)
                  {
                     save_area (1,1,80,23,vwr_buff);
                     DrawFileViewer ( name );
                     vwr_file = fopen ( name, "rb");
                     UseViewer ( c_file.name, c_file.size);
                     fclose (vwr_file);
                     restore_area (1,1,80,23,vwr_buff);
                     free ((char *)vwr_buff);
                  }
              }
              else
                 TextViewer ( name ); /* ASCII Viewer */
          }
          else
             file_not_found();
      }
      restore_area (15,10,66,16,buff);
      free ((char *)buff);
   }
   
}

