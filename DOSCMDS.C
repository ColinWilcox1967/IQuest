#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <dos.h>
#include <bios.h>
#include <process.h>
#include <io.h>
#include <direct.h>
#include <errno.h>
#include "errlist.h"
#include "dostype.h"
#include "palette.h"
#include "iq.h"
#include "windows.h"
#include "keys.h"

extern int dos_param1_type;
extern int dos_param2_type;

extern struct palette_str palette;
extern unsigned char far *video_base;
extern char gen_filename[];
extern char file_list[MAX_FILE_LIST][80];
extern int file_list_count;
extern char current_file[];

extern void ShowEntryStats (char name[]);
extern void show_error (int errnum);
extern unsigned int draw_button (int x, int y, int n, char msg[], int state);
extern void simulate_push(int x, int y, char msg[], int n);
extern unsigned int get_key (void);
extern void establish_dos_types (int cmd, char p1[], char p2[]);
extern unsigned int getstr (int x, int y, char str[], char special[], int width);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void push_wait ( void );
extern void save_env( void );
extern void centre_text (int x1, int x2, int y, char msg[]);
extern void restore_env (void);
extern void hide_cursor( void );
extern void GetFileFromDirectory ( void );
extern void write_blanks ( int x, int y, int n);
extern void centre_push_button ( int x1, int x2, int y, char msg[]);
extern int file_exists ( char name[]);
extern int overwrite_file ( void );

char *dos_action_buff;
char dos_name[67];
char *trans_buff;

void no_wildcards ( void );
void copy_file ( char s[], char t[]);
void get_file ( int x, int y, int len);
int open_dos_buff ( char title[]);
void close_dos_buff ( void );
int get_from_file_list(void);
void dos_copy ( void );
void dos_del ( void );
void dos_ren ( void );
void dos_md ( void );
void dos_rd ( void );
void open_transfer_statics ( void );
void close_transfer_statics ( void );
void write_transfer_status ( char status[]);
void old_file_help(void);
void show_file_list (void);
void show_file_list_entry(int idx, int state);

void show_file_list_entry(int idx, int state)
{
   if (idx >= file_list_count)
   {
      SetColourPair (palette.red, palette.green);
      WriteString (23, 6+idx, "< *** UNDEFINED *** >");
   }
   else
   {
      if (state)
        SetColourPair(palette.yellow, palette.black);
      else
        SetColourPair(palette.black, palette.green);
      WriteString(23,6+idx, strupr(file_list[idx]));
   }
}

void show_file_list()
{
   int idx;
   char str[5];

   for(idx = 0;idx < MAX_FILE_LIST;idx++)
   {
      SetColourPair (palette.bright_white, palette.green);
      sprintf (str, "%02d", idx+1);
      WriteString (18, 6+idx, str);
      show_file_list_entry(idx, FALSE);
   }
}

void old_file_help()
{
  char *old_buff;

  old_buff=(char *)malloc(PORTSIZE(10, 8, 66, 19));
  if(old_buff != NULL)
  {
    save_area(10,8,66,19,old_buff);
    SetColourPair (palette.black, palette.green);
    draw_window(10,8,65,18);
    window_title(10,65,8, "Previous File Help", palette.red, palette.green);
    SetColourPair(palette.black, palette.green);
    WriteString (12, 10, "F1          - Display this help page.");
    WriteString (12, 11, "CSR-UP/DN   - Highlight file");
    WriteString (12, 12, "ALT-I       - Display details on highlighted file.");
    WriteString (12, 13, "'O'/ENTER   - Select highlighted file to view.");
    WriteString (12, 14, "ESC         - Abort selection.");
    centre_push_button (10, 65, 17, "&Ok");
    restore_area(10,8,66,19,old_buff);
    free((char *)old_buff);
  }
}

int get_from_file_list()
{
   char *InfoBuffer;
   int status;
   char *buff;
   int aborted;
   int idx;
   int this_file;
   unsigned int k;

   status = TRUE;
   buff = (char *)malloc(PORTSIZE(15, 5, 61, 20));
   if (buff != NULL)
   {
      save_area(15,5,61,20,buff);
      SetColourPair (palette.black, palette.green);
      draw_window(15,5,60,19);
      window_title(15,60,5, "Previous Viewed Files", palette.red, palette.green);
      SetColourPair (palette.black, palette.green);
      show_file_list();
      aborted=FALSE;
      this_file=0;
      draw_button (25, 18, 4, "&Ok", BUTTON_UP);
      draw_button (45, 18, 8, "&Cancel", BUTTON_UP);
      while (!aborted)
      {
         show_file_list_entry(this_file, TRUE);
         k = get_key();
         if ((k != K_CR) && (k != 'O') && ( k != 'o') &&
             (k != K_ESC) && (k != 'C') && (k != 'c') &&
             (k != K_F1)
            )
         show_file_list_entry(this_file, FALSE);
         switch(k)
         {
            case K_F1     : old_file_help();
                            break;
            case K_ALT_I  :
                            if (file_exists (file_list[this_file]))
                            {
                               InfoBuffer = (char *)malloc (PORTSIZE (5, 3, 76, 12));
                               if (InfoBuffer != NULL)
                               {
                                    save_area (5, 3, 76, 12, InfoBuffer);
                                    SetColourPair (palette.black, palette.green);
                                    draw_window (5, 3, 75, 11);
                                    ShowEntryStats (file_list[this_file]);
                                    restore_area (5, 3, 76, 12, InfoBuffer);
                                    free ((char *)InfoBuffer);
                                }
                            }
                            else
                            {
                               status = FALSE;
                               show_error (ERR_NO_FILE);
                            }
                            break;
            case K_CR     :
            case 'O'      :
            case 'o'      : 
    
                            
                            if (!file_exists (file_list[this_file]))
                                show_error (ERR_NO_FILE);
                            else
                            {
                               aborted = TRUE;
                               simulate_push(25,18, "&Ok", 4);
                               strcpy (current_file, file_list[this_file]);
                            }
                            break;
            case K_ESC    :
            case 'C'      :
            case 'c'      : aborted = TRUE;
                            status = FALSE;
                            simulate_push(45,18,"&Cancel", 8);
                            break;
            case K_CSR_UP : 
                            this_file--;
                            if (this_file<0)
                               this_file=file_list_count-1;
                            break;
            case K_CSR_DN : 
                            this_file++;
                            if (this_file > file_list_count-1)
                               this_file=0;
                            break;
            default       : break;
         }
      }
      restore_area(15,5,61,20,buff);
      free((char *)buff);
   }
   else
     status = FALSE;
   return (status);
}

void open_transfer_statics ()
{
   trans_buff = (char *)malloc(PORTSIZE(15, 16, 66, 23));   
   if (trans_buff != NULL)
   {
      save_area (15, 16, 66, 23, trans_buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_box (15, 16, 65, 21);
      SetColourPair (palette.black, palette.white);
      write_blanks (21, 18, 40);
      SetColourPair (palette.yellow, palette.red);
      WriteString (25, 20, "Status : ");
      SetColourPair(palette.black, palette.red);
      WriteString (21,19,"0%");
      WriteString (57, 19, "100%");
      WriteString (40, 19, "50%");
   }
}

void close_transfer_statics ()
{
   if (trans_buff != NULL)
   {
      restore_area (15, 16, 66, 23, trans_buff);
      free ((char *)trans_buff);
   }
}

void write_transfer_status ( char status[])
{
   SetColourPair (palette.black, palette.red);
   write_blanks (37, 20, 15);
   WriteString (37, 20, status);
}

void no_wildcards ()
{
   char *buff;

   buff = (char *)malloc(PORTSIZE(20, 8, 61, 14));
   if (buff != NULL)
   {
      save_area (20,8,61,14, buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window(20,8,60,13);
      SetColourPair (palette.yellow, palette.red);
      centre_text (21, 60, 10, "Wildcards Not Currently Supported");
      centre_push_button (20, 60, 12, "&Continue");
      restore_area (20,8,61,14, buff);
      free((char *)buff);
   }
}

void show_error ( int error)
{
   char *buff;
   char x[30];

   buff = (char *)malloc(PORTSIZE(20, 8, 61, 14));
   if (buff != NULL)
   {
      save_area (20, 8,61,14, buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window (20, 8,60,13);
      SetColourPair (palette.yellow, palette.red);
      switch (error)
      {
         case ERR_NO_DIR     : centre_text (20,60,10, "Directory Not Found");
                               break;
         case ERR_NO_FILE    : centre_text (20, 60, 10, "File Not Found");
                               break;
         case ERR_NO_SRCE    : centre_text (20,60,10, "Cannot Open Source File");
                               break;
         case ERR_NO_DEST    : centre_text (20,60,10, "Cannot Create Target File");
                               break;
         case ERR_DIR_EXISTS : centre_text (20, 60, 10, "Directory Already Exists");
                               break;
         case ERR_NO_REMOVE  : centre_text (20, 60, 10, "Unable To Remove Directory");
                               break;
         case ERR_TOO_FEW_PARAMS : centre_text(20,60,10, "Not Enough Parameters Specified");
                                   break;
         case ERR_BAD_PATH       : centre_text (20,60,10, "Invalid DOS Path");
                                   break;
         case ERR_BAD_SRCE       : centre_text (20,60,10, "Invalid Source Specified");
                                   break;
         case ERR_BAD_DEST       : centre_text (20,60,10, "Invalid Destination Specified");
                                   break;
         case ERR_NOT_DONE_YET   : centre_text (20,60,10,"Function Not Yet Implemented");
                                   break;
         case ERR_ILLEGAL_STEP   : centre_text (20,60,10, "Illegal Decoding Stage");
                                   break;
         case ERR_USER_TERMINATED : centre_text (20,60,10, "User Has Terminated Process");
                                    break;
         case ERR_UNEXPECTED_ENTRY : centre_text (20,60,10, "Unexpected Data Entered - Aborting");
                                     break;                 
         case ERR_NO_FILE_MEMORY   : centre_text (20,60,10, "Not Enough Memory To Scan Entire Drive");
                                     break;
         default             : sprintf (x, "Unknown DOS Error : %d", error);
                               centre_text (20, 60, 10, x);
                               break;
      }
      centre_push_button (20,60,12, "&Continue");
      restore_area (20,8,61,14,buff);
      free((char *)buff);
   }
}

void copy_file (char srce[], char dest[])
{
    FILE *in,
         *out;
    int ch;
    unsigned long perc;
    struct find_t c_file;
    unsigned long count;
    int           idx;

    open_transfer_statics();
    write_transfer_status ("Opening Files");
    in = fopen (srce, "rb");
    out = fopen (dest, "wb");
    if (in == NULL)
    {
       close_transfer_statics();
       show_error (ERR_NO_SRCE);
       return;
    }
    if (out == NULL)
    {
       close_transfer_statics();
       show_error(ERR_NO_DEST);
       return;
    }
    write_transfer_status ("Copying Data");
    _dos_findfirst (srce, _A_NORMAL , &c_file);
    perc  = (unsigned long)(c_file.size/40L);
    count = 0L;
    idx   = 0;
    while (! feof (in))
    {
       ch = fgetc(in);
       count++;
       if ((count % perc) == 0)
       {
          SetColourPair (palette.black, palette.white);   
          WriteChar (21+idx, 18, 178);
          idx++;
       }
       if (! feof (in))
        fputc (ch, out);
    }
    fclose (in);
    fclose (out);
    write_transfer_status ("Copy Complete");
    close_transfer_statics ();
}

void get_file (int x, int y, int len)
{
    char hotkey[3];
    int state;

    hotkey[0] = (char)K_ESC;
    hotkey[1] = (char)K_ALT_L;
    hotkey[2] = (char)0;
    SetColourPair (palette.bright_white, palette.black);
    state = getstr (x, y, dos_name, hotkey, len);
    if ( state != 0)
    {
        switch ( state )
        {
            case K_ESC   : dos_name[0] = (char)0;
                           break;
            case K_ALT_L : GetFileFromDirectory ();
                           strcpy (dos_name, gen_filename);
                           break;
            default      : break;
        }
    }
}

int open_dos_buff( char title[] )
{
    dos_action_buff = (char *)malloc(PORTSIZE(5, 10, 75, 17));
    if (dos_action_buff == NULL)
        return (0);
    save_area (5, 10, 75, 17, dos_action_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window(5, 10, 74, 16);
    window_title (5, 75, 10, title, palette.red, palette.blue);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 28, 16, " ALT-L For Directory List ");
    return(-1);
}

void close_dos_buff()
{
    restore_area (5, 10, 75, 17, dos_action_buff);
    free((char *)dos_action_buff);
}


void dos_copy ()
{
    char srce[80];
    char dest[80];

    if (! open_dos_buff("DOS Copy"))
        return;
    
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 12, "Source File ");
    WriteString (7, 14, "Target File ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar ( 19, 12, '['); WriteChar (70, 12, ']');
    WriteChar (19, 14, '['); WriteChar (70, 14, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (20, 12, 50); write_blanks (20, 14, 50);
    get_file (20, 12, 50);
    if ( dos_name[0] != (char)0)
    {
        strcpy ( srce, dos_name);
        SetColourPair (palette.bright_white, palette.black);
        WriteString (20, 12, srce);
        get_file (20, 14, 50);
        if (dos_name[0] != (char)0)
        {
            SetColourPair (palette.bright_white, palette.black);
            strcpy ( dest, dos_name);
            WriteString (20, 14, dest);
            if ( strcmpi (srce, dest) == 0)
            {    
                close_dos_buff();
                return;
            }

            establish_dos_types (DOS_COPY, srce, dest);
            if ((dos_param1_type == DOS_WILDCARD) || (dos_param2_type == DOS_WILDCARD))
            {
               no_wildcards();
               close_dos_buff();
               return;
            }
            if ((dos_param1_type == DOS_UNUSED) || (dos_param2_type == DOS_UNUSED))
            {
               show_error (ERR_TOO_FEW_PARAMS);
               close_dos_buff();
               return;
            }

            if ((dos_param1_type == DOS_PATH_ERROR) || (dos_param2_type == DOS_PATH_ERROR))
            {
               show_error (ERR_BAD_PATH);
               close_dos_buff();
               return;
            }

            if (dos_param1_type != DOS_FILE)
            {
               show_error (ERR_BAD_SRCE);
               close_dos_buff();
               return;
            }

            if ((dos_param2_type != DOS_FILE) && (dos_param2_type != DOS_DIRECTORY))
            {
               show_error (ERR_BAD_DEST);
               close_dos_buff();
               return;
            }
            
            /* By this stage all parameters are fine */

            if (dos_param2_type == DOS_DIRECTORY)
            {
               if (dest[strlen(dest)-1] != '\\')
                 strcat (dest, "\\");
               strcat (dest, srce);
            }

            if ( file_exists (srce) && ! file_exists (dest))
                 copy_file (srce, dest);
            else
            {
                if (! file_exists (srce))
                {    
                    show_error (ERR_NO_FILE);
                    hide_cursor();
                }
                else
                {
                    if (file_exists (dest))
                        if ( overwrite_file ())
                        {    
                            copy_file (srce, dest);
                            push_wait();
                            push_wait();
                            push_wait();
                        }
                }
            }
        }
    }
    close_dos_buff();
}

void dos_del ()
{
    if (! open_dos_buff("DOS Delete"))
       return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 13, "FileName ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar (16, 13, '['); WriteChar (67, 13, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (17, 13, 50);
    get_file (17, 13, 50);
    if ( dos_name[0] != (char)0)
    {
        establish_dos_types (DOS_REMOVE, dos_name, dos_name); /* last arg is not used */

        if (dos_param1_type == DOS_WILDCARD)
        {
           no_wildcards();
           close_dos_buff();
           return;
        }
        if (dos_param1_type == DOS_PATH_ERROR)
        {
           show_error (ERR_BAD_PATH);
           close_dos_buff();
           return;
        }

        if (dos_param1_type == DOS_UNUSED)
        {
          show_error (ERR_TOO_FEW_PARAMS);
          close_dos_buff();
          return;
        }
        if (dos_param1_type != DOS_FILE)
        {
           show_error (ERR_BAD_SRCE);
           close_dos_buff();
           return;
        }
        
        if (remove ( dos_name ) != 0)
        {
             switch ( errno )
             {
                case ENOENT  : show_error (ERR_NO_FILE);
                               break;
                case EACCES  : show_error (ERR_NO_REMOVE);
                               break;
                default      : show_error (-999);
                               break;
             }
             hide_cursor();
        }
    }
    close_dos_buff();
}

void dos_ren ()
{
    char oldname[51];
    char newname[51];

    if (!open_dos_buff("DOS Rename"))
       return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 12, "Old Name ");
    WriteString (7, 14, "New Name ");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar ( 16, 12, '['); WriteChar (67, 12, ']');
    WriteChar (16, 14, '['); WriteChar (67, 14, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks(17, 12, 50); write_blanks (17, 14, 50);
    get_file (17, 12, 50);
    if (dos_name[0] != (char)0)
    {
        strcpy (oldname, dos_name);
        get_file (17, 14, 50);
        if ( dos_name[0] != (char)0)
        {
            strcpy ( newname, dos_name);
            if ( strcmpi (oldname, newname) == 0)
            {
                close_dos_buff();
                return;
            }

            establish_dos_types (DOS_RENAME, oldname, newname);
            
            if ((dos_param1_type == DOS_WILDCARD) || (dos_param2_type == DOS_WILDCARD))
            {
               no_wildcards();
               close_dos_buff();
               return;
            }
            if ((dos_param1_type == DOS_UNUSED) || (dos_param2_type == DOS_UNUSED))
            {
               show_error (ERR_TOO_FEW_PARAMS);
               close_dos_buff();
               return;
            }

            if ((dos_param1_type == DOS_PATH_ERROR) || (dos_param2_type == DOS_PATH_ERROR))
            {
               show_error (ERR_BAD_PATH);
               close_dos_buff();
               return;
            }
            if (dos_param1_type != DOS_FILE)
            {
              show_error (ERR_BAD_SRCE);
              close_dos_buff();
              return;
            }

            if (dos_param2_type != DOS_FILE)
            {
              show_error (ERR_BAD_DEST);
              close_dos_buff();
              return;
            }
            
            
            if ( file_exists (oldname) && ! file_exists (newname))
                rename (oldname, newname);
            else
            {
                if (! file_exists (oldname))
                {
                    show_error (ERR_NO_FILE);
                    hide_cursor();
                }
                else
                {
                    if (file_exists (newname))
                    {
                        if (overwrite_file())
                        {
                            remove (newname);
                            rename (oldname, newname);
                        }
                    }
                }
            }
        }
    }
    close_dos_buff();
}

void dos_md ()
{
    if (! open_dos_buff("DOS MkDir"))
        return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 13, "Directory Name");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar ( 22, 13, '['); WriteChar (63, 13, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (23, 13, 40);
    get_file (23, 13, 40);
    if ( dos_name[0] != (char)0)
    {
        establish_dos_types (DOS_CREATE, dos_name, dos_name);
        if (dos_param1_type == DOS_WILDCARD)
        {
           no_wildcards();
           close_dos_buff();
           return;
        }
        if (dos_param1_type == DOS_PATH_ERROR)
        {
           show_error (ERR_BAD_PATH);
           close_dos_buff();
           return;
        }
        if (dos_param1_type == DOS_UNUSED)
        {
           show_error (ERR_TOO_FEW_PARAMS);
           close_dos_buff();
           return;
        }
        if ((dos_param1_type != DOS_DIRECTORY) &&
            (dos_param1_type != DOS_FILE)
           )
        {
          show_error (ERR_BAD_SRCE);
          close_dos_buff();
          return;
        }
        
        
        if (mkdir (dos_name) != 0)
        {
            switch ( errno)
            {
                case EACCES : show_error (ERR_DIR_EXISTS);
                              break;
                case ENOENT : show_error (ERR_NO_DIR);
                              break;
                default     : show_error (-999);
                              break;
            }
            hide_cursor();
        }
    }
    close_dos_buff();
}

void dos_rd ()
{
    if (! open_dos_buff("DOS RmDir"))
        return;
    SetColourPair (palette.yellow, palette.blue);
    WriteString (7, 13, "Directory Name");
    SetColourPair (palette.bright_white, palette.blue);
    WriteChar (22, 13, '['); WriteChar (63, 13, ']');
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (23, 13, 40);
    get_file (23, 13, 40);
    if (dos_name[0] != (char)0)
    {
        establish_dos_types (DOS_REMOVE, dos_name, dos_name);
        if (dos_param1_type == DOS_WILDCARD)
        {
          no_wildcards();
          close_dos_buff();
          return;
        }
        if (dos_param1_type == DOS_PATH_ERROR)
        {
           show_error (ERR_BAD_PATH);
           close_dos_buff();
           return;
        }
        if (dos_param1_type == DOS_UNUSED)
        {
          show_error (ERR_TOO_FEW_PARAMS);
          close_dos_buff();
          return;
        }
        if ((dos_param1_type != DOS_DIRECTORY) &&
            ( dos_param1_type != DOS_FILE)
           )
        {
           show_error (ERR_BAD_SRCE);
           close_dos_buff();
           return;
        }
        
        
        if (rmdir (dos_name) != 0)
        {
            switch ( errno)
            {
                case EACCES : show_error (ERR_NO_REMOVE);
                              break;
                case ENOENT : show_error (ERR_NO_DIR);
                              break;
                default     : show_error (-999);
                              break;
            }
            hide_cursor();
        }
    }
    close_dos_buff();

}

