#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "config.h"
#include "palette.h"
#include "windows.h"
#include "iq.h"
#include "keys.h"

#define ASCII_FORMAT         0
#define HEX_FORMAT           1

#define MAX_PATTERN_LENGTH   10
#define MAX_ASCII_LENGTH     MAX_PATTERN_LENGTH
#define MAX_HEX_LENGTH       3 * MAX_PATTERN_LENGTH

char pattern[MAX_PATTERN_LENGTH+1];
int  segment_size;

extern unsigned char config_byte1;
extern struct palette_str palette;

extern void TextSearch (char name[], int mode);
extern int abandon_option_changes( void );
extern unsigned int getstr (int x, int y, char str[], char special[], int length);
extern void push_wait ( void );
extern void simulate_push (int x, int y, char msg[], int width);
extern unsigned int  draw_button (int x, int y, int width, char msg[], int state);
extern void SetColourPair (short, short);
extern void draw_edging (int x1, int y1, int x2, int y2);
extern void write_blanks ( int x, int y, int n);
extern void clear_area (int, int, int,  int);
extern void draw_window (int, int, int, int);
extern void window_title ( int x1, int x2, int y, char msg[], short fg, short bk);
extern void WriteChar (int, int, char);
extern void WriteString (int, int, char *);
extern unsigned int get_key (void);
extern void save_area (int, int, int, int, char *);
extern void restore_area (int,  int, int,  int, char *);
extern void WriteMultiChar (int, int, char, int);
extern void centre_push_button ( int x1, int x2, int y, char msg[]);

int segment[270];
int new_segment[270];

FILE *vwr_file;
int  vwr_char;

int edit_file (int frame_base);
void DrawFileViewer (char *);
void Num2Hex (int, char *);
void ShowFileSegment (long, long);
void ShowPatternFormats ( void );
void UseViewer (char name[], long size);
void do_pattern_search ( void );
void show_current_byte ( int offset, int state);
void show_edit_range (int offset);
void show_active_byte (int start, int offset, int state);
void file_edit_help ( void );

void ShowPatternFormats()
{
   int i;
   char HexByte[3];

   SetColourPair (palette.bright_white, palette.black);
   for (i = 0; i < strlen (pattern); i++)
   {
       WriteChar (30+i, 9, pattern[i]);
       Num2Hex ((int)pattern[i], HexByte);
       WriteString (30 + (3 * i), 11, HexByte); 
       WriteChar (30 + (3 * i) + 2, 11, ' ');
   }
}

void show_edit_range (int start )
{
   int idx,
       from, 
       to;
   char hex[3];

   from = start;
   to = start+10;
   to = (to > segment_size ? segment_size:to);
   for (idx=from; idx<=to; idx++)
   {
      SetColourPair (palette.bright_white, palette.blue);
      Num2Hex (segment[idx], hex);
      WriteString (29, (idx-from)+6, hex);
      WriteChar (35, (idx-from)+6, (char)segment[idx]);
      SetColourPair (palette.cyan, palette.blue);
      Num2Hex (new_segment[idx], hex);
      WriteString (45, (idx-from)+6, hex);
      WriteChar (51, (idx-from)+6, (char)new_segment[idx]);
   }

   SetColourPair (palette.red, palette.blue);
   WriteChar (53, 6, ' '); WriteChar (53, 16, ' ');
   if (from >= 10)
     WriteChar (53, 6, (char)24);
   if (to+10 < segment_size)
     WriteChar (53, 16, (char)25);
}

void file_edit_help ()
{
  char *buff;

  buff = (char *)malloc(PORTSIZE(15, 7, 66, 19));
  if (buff != NULL)
  {
    save_area (15, 7, 66, 19, buff);
    SetColourPair (palette.black, palette.green);
    draw_window (15, 7, 65, 18);
    window_title (15, 65, 7, "Edit File Help", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString (18,  9, "F1         - Display This Help Page.");
    WriteString (18, 10, "CSR-UP/DN  - Step Through File In Bytes.");
    WriteString (18, 11, "PGUP/PGDN  - Page Through File In 10 Byte"); 
    WriteString (18, 12, "             Blocks.");
    WriteString (18, 13, "CR/O       - Accept Changes To File.");
    WriteString (18, 14, "ESC        - Abandon Changes.");
    WriteString (18, 15, "Other Keys - Used To Change Highlighted Byte.");
    centre_push_button (15, 65, 17, "&Continue");
    restore_area (15, 7, 66, 19, buff);
    free ((char *)buff);
  }
}

void show_active_byte (int base, int offset, int state)
{
   char hex[3];

   if (state)
      SetColourPair (palette.bright_white, palette.black);
   else
     SetColourPair (palette.cyan, palette.blue);
   Num2Hex (new_segment[base+offset], hex);
   WriteString (45, 6+offset, hex);
   WriteChar (51, 6+offset, (char)new_segment[base+offset]);
}

int edit_file (int frame_base)
{
   char         *editbuff;
   int          position;
   int          finish;
   int          frame_offset;
   int          changes_made;
   unsigned int key;
   char         hotkeys[5];
   char         str[3];

   memcpy (&new_segment[0], &segment[0], sizeof (new_segment));
   editbuff = (char *)malloc(PORTSIZE(25, 2, 56, 21));
   if (editbuff != NULL)
   {
      position = 0;
      frame_offset = frame_base;
      save_area (25,2,56,21,editbuff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window(25,2,55,20);
      window_title (25,55,2,"Edit File Details", palette.red, palette.blue);
      SetColourPair (palette.yellow, palette.blue);
      WriteString (28, 4, "Old Value");
      WriteString (44, 4, "New Value");
      draw_button (32, 18, 4, "&Ok", BUTTON_UP);
      draw_button (41, 18, 8, "&Cancel", BUTTON_UP);
      show_edit_range (position+frame_base);
      finish = FALSE;
      changes_made=FALSE;
      while (! finish)
      {
	 show_active_byte ( frame_offset, position, TRUE );
	 key = get_key();
	 show_active_byte (frame_offset, position, FALSE);
	 switch ( key )
	 {
	    case K_F1     : file_edit_help();       
			    break;
	    case K_CSR_UP : if (position > 0)
			      position--;
			    else
			    {
			       if (frame_offset > 9)
			       {
				  frame_offset -=10;
				  position = 9;
				  show_edit_range(position+frame_base);
			       }
			    }
			    break;
	    case K_CSR_DN : if (position < 9)
				position++;
			    else
			    {
			       if (frame_offset+10 < segment_size)
			       {
				  frame_offset+=10;
				  position=0;
				  show_edit_range(position+frame_base);
			       }
			    }
			    break;
	    case K_PGUP   :
			    break;
	    case K_PGDN   :
			    break;

	    case 'O'   :
	    case 'o'   :
	    case K_CR  :
			 finish = TRUE;
			 simulate_push (32, 18, "&Ok", 4);
			 push_wait();
			 changes_made=TRUE;
			 memcpy (&segment[0], &new_segment[0], sizeof (segment));
			 break;
	    case K_ESC : 
	    case 'C'   : 
	    case 'c'   : simulate_push (41, 18, "&Cancel", 8);
			 push_wait();
			 finish = TRUE;
			 if (memcmp (&new_segment[0], &segment[0], sizeof (segment) != 0))
			 {
			    if (!abandon_option_changes ())
			    {
			       memcpy (&segment[0], &new_segment[0], sizeof (segment));
			       changes_made=TRUE;
			    }
			 }
			   
			 break;
	    default    : hotkeys[0] = K_ESC;
			 hotkeys[1] = K_CSR_UP;
			 hotkeys[2] = K_CSR_DN;
			 hotkeys[3] = K_TAB;
			 hotkeys[4] = (char)0;
			 SetColourPair (palette.bright_white, palette.black);
			 write_blanks (51, 6+position, 2);
			 key = getstr (51, 6+position, str, hotkeys, 2);
			 if (key == 0)
			   new_segment[position+frame_base] = atoi (str);
			 else
			 {
			     switch (key)
			     {
				 case K_ESC  : finish = TRUE;
					       simulate_push (41, 8, "&Cancel", 8);
					       push_wait();
					       break;
				 case K_CSR_UP : 
				 case K_REVTAB : if (position > 0)
						   position--;
						 else
						 {
						    if (frame_offset > 9)
						    {
						       position = 9;
						       frame_offset -=10;
						    }
						 }
						 break;
				 case K_CSR_DN : 
				 case K_TAB    : if (position < 9)
						    position++;
						 else
						 {
						    if (frame_offset+10 < segment_size)
						    {
						       frame_offset +=10;
						       position=0;
						    }
						 }
						 break;
			     }
			 }
			 break;
	 }
      }
      restore_area (25,2,56,21,editbuff);
      free((char *)editbuff);
   }
   return(changes_made);
}

void DrawFileViewer (char *f)
{
    int  i;

    SetColourPair (palette.black, palette.cyan);
    
    draw_edging (6, 1, 73, 22);
    if ( config_byte1 & MSK_SHADOW_TYPE)
       draw_shadow (6,1,73,22);
    if (config_byte1 & MSK_BORDER_TYPE)
    {
       WriteChar (6, 3, (char)218);
       WriteChar (73, 3, (char)191);
       for(i=7;i<=72;i++)
	 WriteChar (i, 3, (char)196);
       WriteChar (6, 3, (char)195); 
       WriteChar (73, 3, (char)180);
       WriteChar (54, 22, (char)193);
       for (i=4; i<=21; i++)
	  WriteChar (54, i, (char)179);
       WriteChar (54, 3, (char)194);
    }
    else
    {
       WriteChar (6, 3, (char)204);
       WriteChar (73, 3, (char)185);
       for (i=7; i <= 72; i++)
	   WriteChar (i, 3, (char)205);
       WriteChar (6, 3, (char)204);
       WriteChar (73, 3, (char)185);
       WriteChar (54, 22, (char)202);
       for (i = 4; i <= 21; i++)
	  WriteChar (54, i, (char)186);
       WriteChar (54, 3, (char)203);
    }
    SetColourPair (palette.red, palette.cyan);
    WriteString ( 8,2 ,strupr(f));
}

char hex_digit (int n)
{
    if (n > 9)
	return ((char)('A' + (n - 10)));
    else
	return ((char)('0' + n));
}

void Num2Hex (int n, char *hex)
{
    hex[0] = hex_digit (n / 16);
    hex[1] = hex_digit (n % 16);
    hex[2] = (char)0;
}

void show_current_byte ( int x, int state )
{
   char hex[3];
   int  row, column1, column2, idx;

   Num2Hex ( segment[x], hex);

   row = 4;
   column1 = 8; column2 = 56;
   idx=0;
   while (idx < x)
   {
      idx++;
      column1 += 3; 
      column2++;
      if ((idx % 15) == 0)
      {
	 row++;
	 column1=8;
	 column2=56;
      }
   }
   if (!state)
      SetColourPair (palette.bright_white, palette.cyan);
   else
      SetColourPair (palette.yellow, palette.red);

   WriteString (column1, row, hex);
   WriteChar (column2, row, (unsigned char)segment[x]);

}

void ShowFileSegment (long page_number, long totalpages)
{
    int  count;
    unsigned  int  line,
		   column1,
		   column2;
    char hex[3];
    char page1[4], page2[4];
    char tmp[80];

    count        = 0;
    segment_size = 0;
    line         = 4;
    column1 = 8; column2 = 56;

    SetColourPair (palette.black, palette.cyan);

    clear_area (7, 4, 53, 21);
    clear_area (55, 4, 71, 21);

    WriteMultiChar (48, 3, ' ', 6);

    SetColourPair (palette.red, palette.cyan);
    strcpy (tmp, " Page ");
    ltoa (page_number, page1, 10);

    strcat (tmp, page1);
    strcat (tmp, "/");
    ltoa (totalpages, page2, 10);
    strcat (tmp, page2);
    strcat (tmp, " ");

    WriteString (55 - strlen(tmp), 3, tmp);

    SetColourPair (palette.bright_white, palette.cyan);
    while ((count < 270L) && ! feof (vwr_file))
    {
	if (! feof (vwr_file))
	{     
	    vwr_char = fgetc (vwr_file);
	    segment[count] = vwr_char;
	    count++;
	    Num2Hex (vwr_char, hex);

	    WriteString (column1, line, hex);
	    WriteChar   (column1 + 2, line, ' ');
	    WriteChar   (column2, line, (char)vwr_char);
 
	    column1 += 3;
	    column2++;
 
	    if ((count % 15) == 0L)
	    {
		line++;
		column1 = 8;
		column2 = 56; 
	    }
	}
    }           
    segment_size = count;
}

int HexDigit (char ch)
{
    if ((ch >= 'A') && (ch <= 'F'))
	return ( 10 + ( ch-'A'));
    else
	return ( ch - '0');
}

int HexToDec (char hex[])

{
    return ( 16 * HexDigit (hex[0])+HexDigit(hex[1]));    
}

int get_n_chars (int x, int y, char *str, int n)
{
    int done,
	retval,
	i;
    unsigned int ch;

    memset ( str, 0, n);
    i = 0;
    done = FALSE;
    retval = 0; /* OK, no error */
    while ( ! done )
    {
       ch = get_key();
       str[i++]=(char)ch;
       if (i == n)
       {
	  str[i-1]=(char)0;
	  done = TRUE;
	  retval=1; /* ok n chars read */
       }
       else
       {
	   done = ((ch == K_ESC) || ( ch == K_TAB));
	   if ( done )
	   {
	       if (ch == K_TAB)
		  retval = -2;  /* tab pressed */
	       else
		  retval = -1;   /* esc pressed */
	   }
       }
       if (! done)
       {
	   SetColourPair (palette.bright_white, palette.black);
	   WriteString (x, y, str);
       }
    }
    return (retval);
}


void do_pattern_search ()
{
     char *pattern_buff;
     int  finished;
     int  format_type;
     int  state, idx;
     char ch_str[3];
     char hotkeys[3];

     pattern_buff = (char *)malloc(PORTSIZE(17, 7, 64, 14));
     if ( pattern_buff == NULL)
	 return;
     save_area (17, 7, 64, 14, pattern_buff);
     SetColourPair (palette.bright_white, palette.blue);
     draw_window (17, 7, 63, 13);
     SetColourPair (palette.yellow, palette.blue);
     WriteString ( 20,  9, "ASCII    [");
     WriteString ( 20, 11, "Hex      [");
     WriteChar ( 30+MAX_ASCII_LENGTH, 9, ']');
     WriteChar ( 30+MAX_HEX_LENGTH, 11, ']');
     SetColourPair (palette.bright_white, palette.black);
     write_blanks ( 30, 9, MAX_ASCII_LENGTH);
     write_blanks (30, 11, MAX_HEX_LENGTH);
     finished = FALSE;
     format_type = ASCII_FORMAT;
     hotkeys[0] = (char)K_ESC; hotkeys[1] = (char)K_TAB; hotkeys[2] = (char)0;
     idx = 0;
     memset (pattern, 0, sizeof (pattern)); /* zero fill receipt buffer */
     finished = FALSE;
     while (! finished)
     {
	 ShowPatternFormats();
	 SetColourPair (palette.bright_white, palette.black);
	 if ( format_type == ASCII_FORMAT )
	     state = get_n_chars (30, 9, ch_str, 1);
	 else
	     state = get_n_chars (30, 11, ch_str, 2);
	 switch ( state )
	 {
	     case -1    : finished = TRUE;
			  break;
	     case -2    : if ( format_type == ASCII_FORMAT )
			     format_type = HEX_FORMAT;
			  else
			     format_type = ASCII_FORMAT;
			  break;
	     default    : if ( format_type == ASCII_FORMAT )
			       pattern[idx++] = ch_str[0];
			  else
			       pattern[idx++] = (char)HexToDec (ch_str);
			  break;
	 }
     }
     restore_area (17, 7, 64, 14, pattern_buff);
     free ((char *)pattern_buff);
}

void UseViewer (char name[], long FileSize)
{
    unsigned int ch;
    int          abort;
    char         *InfoBuffer;
    long         pages;
    long         TotalPages;
    int          offset;
    
    abort = 0;
    pages = 1L;
    offset = 0;

    TotalPages = (FileSize / 270L)+1;
    ShowFileSegment (pages, TotalPages);
    show_current_byte (offset, TRUE);
    while (! abort)
    {
	SetColourPair (palette.bright_white, palette.cyan);
	ch = get_key();
	switch (ch)
	{
	   /*    case K_ALT_S  : TextSearch (name, config_byte1 & MSK_FILE_VIEWER);
			       break;  */
	       case K_ESC    : abort = TRUE;
			       break;
	       /* case K_CR     : if (edit_file (offset))
			       {
				    TotalPages = (FileSize / 270L)+1;
				    ShowFileSegment (pages, TotalPages);
			       }
			       break; */
	       case K_CSR_DN : if ( offset + 15 < segment_size)
			       {
				  show_current_byte (offset, FALSE);
				  offset+=15;
				  show_current_byte (offset, TRUE);
			       }
			       else
			       {
				  if (pages < TotalPages)
				  {
				      pages++;
				      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				      ShowFileSegment (pages, TotalPages);
				      offset = (offset % 15);
				      show_current_byte (offset, TRUE);
				  }
			       }
			       break;
	       case K_CSR_UP : if (offset >= 15)
			       {
				  show_current_byte (offset, FALSE);
				  offset-=15;
				  show_current_byte(offset, TRUE);
			       }
			       else
			       {
				  if (pages > 1L)
				  {
				    pages--;
				    rewind (vwr_file);
				    fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				    ShowFileSegment (pages, TotalPages);
				    offset = (255 + (offset % 15));
				    show_current_byte (offset, TRUE);
				  }
			       }
			       break;
	       case K_CSR_RT : if (offset < segment_size-1)
			       {
				  show_current_byte ( offset, FALSE);
				  offset++;
				  show_current_byte (offset, TRUE);
			       }
			       else
			       {
				  if (pages < TotalPages)
				  {
				      pages++;
				      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				      ShowFileSegment (pages, TotalPages);
				      offset = 0;
				      show_current_byte (offset, TRUE);
				  }
			       }
			       break;
	       case K_CSR_LT  : if (offset > 0)
				{
				   show_current_byte (offset, FALSE);
				   offset--;
				   show_current_byte (offset, TRUE);
				}
				else
				{
				   if (pages > 1L)
				   {
				      pages--;
				      rewind (vwr_file);
				      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				      ShowFileSegment (pages, TotalPages);
				      offset = 269;
				      show_current_byte (offset, TRUE);
				   }
				}
				break;
	       case K_PGUP : 
			       if (pages > 1L)
			       {
				    pages--;
				    rewind (vwr_file);
				    fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				    ShowFileSegment (pages, TotalPages);
				    show_current_byte (offset, TRUE);
			       }
			       break;
		case K_PGDN : 
				if (pages < TotalPages)
				{
				      pages++;
				      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
				      ShowFileSegment (pages, TotalPages);
				      if (offset > segment_size)
					 offset = 0;
				      show_current_byte (offset, TRUE);
				}
				break;

		case K_HOME  : fseek (vwr_file, 0L, SEEK_SET);
				ShowFileSegment (1L, TotalPages);
				pages = 1L;
				break;

		case K_END  : rewind (vwr_file);
			      pages = TotalPages;
			      fseek (vwr_file, (long)((pages-1)*270L), SEEK_SET);
			      ShowFileSegment (TotalPages, TotalPages);
			      break;

	       case K_F1  : InfoBuffer = (char *)malloc (PORTSIZE(15, 4, 66, 17));
			    if (InfoBuffer != NULL)
			    {
				  save_area (15, 4, 66, 15, InfoBuffer);
				  SetColourPair (palette.black, palette.green);
				  draw_window (15, 4, 65, 14);
				  window_title (15, 65, 4, "File Viewer Help",palette.red, palette.green);
				  SetColourPair (palette.black, palette.green);
				  WriteString (18,   6, "F1               - Display This Help Screen.");
				  WriteString (18,   7, "ESC              - Abort File Viewer.");
				  WriteString (18,   8, "HOME             - Jump To First Page.");
				  WriteString (18,   9, "END              - Jump To Last Page.");
				  WriteString (18,  10, "PGUP/DN          - Page Through File.");
				  WriteString (18,  11, "Csr Keys         - Move Around File.");
				  /* WriteString (18,  13, "ENTER            - Edit File Segment."); */

				  SetColourPair (palette.black, palette.green);
				  centre_push_button (15, 64,13, "&Continue"); 
				  restore_area (15, 4, 66, 15, InfoBuffer);
				  free ((char *)InfoBuffer);
			   }               
			   break;

	       default   : break;
	}
    }
}
