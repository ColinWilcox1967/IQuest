#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <process.h>
#include <string.h>
#include <errno.h>
#include "iq.h"
#include "palette.h"
#include "windows.h"

#define COMMAND    "C:\\COMMAND.COM"
#define SHELL_MSG1 "*** IQuest v1.1 - DOS Shell"
#define SHELL_MSG2 "*** Type EXIT To Return To Application."

extern void GotoXY (int x, int y);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void centre_push_button (int x1, int x2, int y, char msg[]);

void save_area     (int x1, int y1, int x2, int y2, char *buff);
void restore_area  (int x1, int y1, int x2, int y2, char *buff);
void SetColourPair (short fg, short bk);
void clear_area    (int x1, int y1, int x2, int y2);

extern struct palette_str palette;

char *save_screen;

void do_shell ( void );
void shell_error ( int errornum );

void shell_error ( int errornum )
{
   char *buff;
   char msg[80];

   buff= (char *)malloc(PORTSIZE(20, 8, 61, 14));
   if (buff != NULL)
   {
      save_area (20,8,61,14,buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window(20,8,60,13);
      window_title (20,60,8, "SHELL Error", palette.bright_white, palette.red);
      SetColourPair (palette.yellow, palette.red);
      switch (errornum)
      {
         case E2BIG   : strcpy (msg, "Argument List Is Too Long");
                        break;
         case EINVAL  : strcpy (msg, "Invalid Spawn Mode Selected");
                        break;
         case ENOENT  : strcpy (msg, "Unable To Find Command Interpreter");
                        break;
         case ENOEXEC : strcpy (msg, "Spawn Program Is Not An Executable");
                        break;
         case ENOMEM  : strcpy (msg, "Insufficient Memory To Perform Shell");
                        break;
         default      : sprintf (msg, "Unknown Spawning Error : %d", errno);
                        break;
      }
      centre_text (21, 60, 10, msg);
      centre_push_button (21,60, 12, "&Continue");
      restore_area (20,8,61,14,buff);
      free((char *)buff);
   }
}

void do_shell ()
{
   int status;

   save_screen = (char *)malloc(PORTSIZE(1,1,80,25));
   if (save_screen != NULL)
   {
      save_area (1,1,80,25,save_screen);
      SetColourPair (palette.white, palette.black);
      clear_area (1,1,80,25);
      WriteString (1,1, SHELL_MSG1);
      WriteString (1,2, SHELL_MSG2);
      GotoXY (1, 4);
      status = spawnl (P_WAIT, COMMAND, NULL);
      restore_area (1,1,80,25,save_screen);
      free((char *)save_screen);
      if (status == -1) /* Oh dear */
         shell_error (errno);
   }
}
