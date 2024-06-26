#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <dos.h>
#include <conio.h>
#include "keys.h"
#include "iq.h"
#include "config.h"
#include "windows.h"
#include "palette.h"

extern int segment[270];
extern int new_segment[270];
extern struct palette_str palette;
extern unsigned char config_byte1;

#define MAX_SEARCH_STRING 30

extern void show_active_byte (int base, int ofs, int state);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void centre_push_button (int x1, int x2, int y, char msg[]);

extern void         centre_text (int x1, int x2, int y, char msg[]);
extern void         push_wait(void);

extern ShowFileSegment (long segment, long total_pages);

extern void         show_current_byte (long ofs, int n);
extern void         write_blanks (int, int, int);
extern unsigned int draw_button(int x, int y, int n, char msg[], int state);
extern void         simulate_push (int x, int y, char msg[], int n);
extern int          same_char (char, char);
extern void         SetColourPair (short, short);
extern void         clear_area (int, int, int, int);
extern unsigned int get_key (void);
extern void         WriteChar (int, int, char);
extern void         WriteString (int, int, char *);
extern void         WriteMultiChar (int, int, char, int);
extern unsigned int getstr (int, int, char *, char *, int);
extern void         save_area (int, int, int, int, char *);
extern void         restore_area (int, int, int, int, char *);
extern void         draw_window (int, int, int, int);
extern void         hide_cursor (void);
extern char         *CommaPad (char *);

struct SrchLst
{
    long           SrchOffset;
    struct SrchLst *PreviousMatch;
    struct SrchLst *NextMatch;
};

long TotalMatches;
struct SrchLst *SrchList;

void InitSrchLst (void);
void ClearSrchLst (void);
void AddMatch (long);


void TextSearch (char name[], int mode_of_operation);
void FindMatches (char *, char *);
void ShowMatches (FILE *, char *);
void ShowMatchContext (FILE *, long, char *);
void HexSearch (char FileSpec[], char SearchString[]);
void ShowHexMatches ( long pages, char string[]);
void hex_search_help ( void );
void highlight_segment_bytes (long base, int ofs, int n);

int  same_char (char, char);
 
/* 'mode_of_operation' - non-zero set by Hex Viewer State - 
			 Dont Call FindMatches */

void highlight_segment_bytes ( long base, int offset, int n)
{
   int i;

   for (i=0; i<n;i++)
      show_active_byte ((int)base, offset, TRUE);
}

void hex_search_help()
{
   char *buff;

   buff = (char *)malloc(PORTSIZE(15, 8, 66, 17));
   if (buff != NULL)
   {
     save_area(15,8,66,17,buff);
     SetColourPair (palette.black, palette.green);
     draw_window(15,8,65,16);
     window_title (15,65,8,"Hex Search Help", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (18, 10, "F1        - Display this help page.");
     WriteString (18, 11, "PGUP/DN or ");
     WriteString (18, 12, "CSR-UP/DN - List through search matches.");
     WriteString (18, 13, "ESC       - Quit back to viewer");
     centre_push_button (15,65, 15, "&Continue");
     restore_area(15,8,66,17,buff);
     free((char *)buff);
   }
}

void ShowHexMatches (long TotalPages, char SearchString[] )
{
   int          aborted,
		refresh;
   unsigned int key;
   unsigned int match;
   struct SrchLst *ptr;
   char str[80];
   char *buff;
   long srch_start;
   long found_in_page;

   aborted = FALSE;
   match=1;
   ptr = SrchList;
   if (ptr == NULL) /* No Matches */
   {
      aborted = TRUE;
      buff = (char *)malloc(PORTSIZE(25, 8, 56, 13));
      if(buff != NULL)
      {
	save_area(25,8,56,13,buff);
	SetColourPair(palette.bright_white, palette.blue);
	draw_window(25,8,55,12);
	SetColourPair(palette.yellow, palette.blue);
	centre_text (25, 55, 9, "No Matches Found");
	centre_push_button (25, 55, 11, "&Continue");
	restore_area(25,8,56,13,buff);
	free((char *)buff);
      }
   }
   refresh = TRUE;
   while (! aborted)
   {

      if (refresh)
      {
	 found_in_page = (ptr->SrchOffset / 270L);
	 ShowFileSegment (found_in_page+1L, TotalPages);
	 srch_start = found_in_page;
      
	 sprintf (str, "Page# = %d Ofset = %ld   Total = %ld", found_in_page+1L, (ptr->SrchOffset - (270L*found_in_page)), TotalPages);   
	 WriteString (50, 1, str);
	 WriteString (30, 1, SearchString);

	 memcpy (&new_segment[0], &segment[0], sizeof(new_segment));
	 highlight_segment_bytes (found_in_page, (int)(ptr->SrchOffset-(270L*found_in_page)), strlen(SearchString));
      }
      refresh = TRUE;
      key = get_key();
      switch (key )
      {
	case K_F1     : hex_search_help();
			refresh=FALSE;
			break;
	case K_ESC    : aborted = TRUE;
			break;
	case K_PGDN   :
	case K_CSR_DN : if (ptr->NextMatch != NULL)
			{
			   match++;
			   ptr=ptr->NextMatch;
			}
			else
			  refresh=FALSE;
			break;
	case K_PGUP  :
	case K_CSR_UP : if (ptr->PreviousMatch != NULL)
			{
			   match--;
			   ptr=ptr->PreviousMatch;
			}
			else
			  refresh=FALSE;
			break;
	default       : refresh = FALSE;
			break;
      }
   }
}

void HexSearch (char FileSpec[], char SearchString[])
{
   char          *buff;
   int           PatternLength,
		 ch,
		 StillMatchingBytes,
		 AbortSearch;
   struct find_t c_file;
   long          FileLength,
		 FileOffset,
		 FilePosition;
   int           count;
   FILE          *FilePtr;

   buff = (char *)malloc(PORTSIZE(25, 8, 56, 14));
   if (buff!= NULL)
   {
      save_area(25,8,56,14,buff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (25,8,55,13);
      SetColourPair (palette.yellow, palette.blue);
      centre_text(26,55,10, "Searching For Matches");
      draw_button(37,12, 6, "&Stop", BUTTON_UP);
   }
   _dos_findfirst (FileSpec, FILE_ATTR_LIST, &c_file);
   FileLength = c_file.size;

   PatternLength = (int)strlen (SearchString);
   FilePosition  = 0L;
   AbortSearch = 0;
   InitSrchLst ();

   FilePosition=0L;
   FilePtr = fopen (FileSpec, "r+");

   while ((FilePosition < FileLength - (long)PatternLength) && (! AbortSearch))
   {
       FileOffset = 0;
       StillMatchingBytes = TRUE;

       while ((FileOffset < PatternLength) && (StillMatchingBytes) && (! AbortSearch))
       {
	  if (kbhit ())
	      AbortSearch = (get_key () == K_ESC);

	  rewind (FilePtr);
	  fseek (FilePtr, FilePosition + FileOffset, SEEK_SET);
	  ch = fgetc (FilePtr);

	  StillMatchingBytes = same_char ((char)ch, SearchString[FileOffset]);

	  if (StillMatchingBytes)
		FileOffset++;
       }

       if (StillMatchingBytes)
       {
	     AddMatch (FilePosition);
	     count++;
       }
       FilePosition++;
   }
   if (AbortSearch)
   {
      simulate_push(37,12,"&Stop", 6);
      push_wait();
   }
   
   if (buff != NULL)
   {
      restore_area(25,8,56,14,buff);
      free((char *)buff);
   }

   /* Built Search List Now Display */
   
   ShowHexMatches ((long)((FileLength/270L)+1L), SearchString);

   ClearSrchLst();
   fclose(FilePtr);
}

void TextSearch (char *FileSpec, int mode_of_operation)
{
    char *WinBuffer;
    char SearchString[MAX_SEARCH_STRING + 1];
    char HotKeys[2];
    char ReturnKey;

    HotKeys[0] = (char)27;   /* Allow ESCape as a means of terminating string entry */
    HotKeys[1] = (char)0;

    WinBuffer = (char *)malloc (PORTSIZE(23, 6, 57, 12));
    if (WinBuffer != NULL)
    {
	save_area (23, 6, 57, 12, WinBuffer);
	SetColourPair (palette.bright_white, palette.blue);
	draw_window (23, 6, 56, 11);
	SetColourPair (palette.yellow, palette.blue);
	WriteString (30, 7, "Enter Search Pattern");
	SetColourPair (palette.bright_white, palette.black);
	WriteMultiChar (25, 9, ' ', MAX_SEARCH_STRING);
	ReturnKey = (char)getstr (25, 9, SearchString, HotKeys, MAX_SEARCH_STRING);
	hide_cursor;        
	restore_area (23, 6, 57, 12, WinBuffer);
	free ((char *)WinBuffer);
	
	if(! mode_of_operation ) /* ASCII Viewer Case */
	{
	    if ((ReturnKey != HotKeys[0]) && (strlen (SearchString) > 0))
	    {
		FindMatches (FileSpec, SearchString);
	    }
	}
	else
	{
	   /* Hex Search Case */

	   if ((ReturnKey != HotKeys[0]) && (strlen(SearchString) >0))
	   {
	      HexSearch(FileSpec, SearchString);
	   }
	}

    }
}


void ShowSearchWindow (char *filename)
{
     SetColourPair (palette.black, palette.green);
     draw_window (2, 3, 77, 21);
     if (config_byte1 & MSK_BORDER_TYPE)
     {
	 WriteChar (2, 5, (char)195);
	 WriteChar (77, 5, (char)180);
	 WriteMultiChar (3, 5, (char)196, 74);
	 WriteChar (17, 3, (char)194);
	 WriteChar (17, 5, (char)193);
	 WriteChar (17, 4, (char)179); 
	 WriteChar (2, 9, (char)195);
	 WriteChar (77, 9, (char)180);
	 WriteMultiChar (3, 9, (char)196, 74);

     }
     else
     {
	 WriteChar (2, 5, (char)204);
	 WriteChar (77, 5, (char)185);
	 WriteMultiChar (3, 5, (char)205, 74);
	 WriteChar (17, 3, (char)203);
	 WriteChar (17, 5, (char)202);
	 WriteChar (17, 4, (char)186);
	 WriteChar (2, 9, (char)204);
	 WriteChar (77, 9, (char)185);
	 WriteMultiChar (3, 9, (char)205, 74);
     }
     WriteString (19, 4, "Searching [");
     WriteChar (76, 4, ']');
     SetColourPair (palette.red, palette.green);
     WriteString (4, 4, filename);
     WriteString ( 30, 21, " Press F1 For Help ");
     SetColourPair (palette.yellow, palette.red);
     write_blanks (30, 4, 46);
}

void FindMatches (char *FileSpec, char *SearchString)
{
    long          FilePosition,
		  FileLength;
    int           FileOffset,
		  ArrowPosition,
		  PatternLength,
		  ArrowDelay,
		  AbortSearch,
		  StillMatchingBytes;
    struct find_t c_file;
    FILE          *FilePtr;
    char          *InfoBuffer;
    char          *InfoBuffer3;
    int           ch;
    char          str[30];
    long          count;

    InfoBuffer  = (char *)malloc (PORTSIZE(2, 3, 78, 22));
    InfoBuffer3 = (char *)malloc (PORTSIZE(21, 14, 60, 19));
    if ((InfoBuffer != NULL) && (InfoBuffer3 != NULL))
    {
	ArrowPosition = 30;  
	ArrowDelay = 0;       
	count = 0L;           

	_dos_findfirst (FileSpec, FILE_ATTR_LIST, &c_file);
	FileLength = c_file.size;
 
	save_area (2, 3, 78, 22, InfoBuffer);
	ShowSearchWindow (c_file.name);
 
	PatternLength = (int)strlen (SearchString);
	FilePosition  = 0L;
	AbortSearch = 0;
	InitSrchLst ();

	SetColourPair (palette.bright_white, palette.green ); /* flash the white */ 
	WriteString ( 30, 9, " Press ESC To Abort ");
	SetColourPair (palette.red, palette.green);
	WriteString ( 37, 9, "ESC");

	SetColourPair (palette.bright_white, palette.blue);
	save_area (21, 14, 60, 19, InfoBuffer3);
	draw_window (21, 14, 59, 18);
	SetColourPair (palette.yellow, palette.blue);
	WriteString (23, 16, "Matches Found So Far = 0");

	FilePtr = fopen (FileSpec, "r+");
	while ((FilePosition < FileLength - (long)PatternLength) && (! AbortSearch))
	{
	    FileOffset = 0;
	    StillMatchingBytes = TRUE;

	    while ((FileOffset < PatternLength) && (StillMatchingBytes) && (! AbortSearch))
	    {
		   if (kbhit ())
			 AbortSearch = (get_key () == K_ESC);

		   ArrowDelay++;
		   if (ArrowDelay == 100)
		   {              
			  ArrowPosition++;
			  ArrowDelay = 0;
		   }
	
		   if (ArrowPosition > 75)
		   {
			  ArrowPosition = 30;
			  SetColourPair (palette.black, palette.red);
			  write_blanks (30, 4, 46);
		   }

		   SetColourPair (palette.black, palette.bright_white);
		   WriteChar (ArrowPosition, 4, (char)175);
		   rewind (FilePtr);
		   fseek (FilePtr, FilePosition + FileOffset, SEEK_SET);
		   ch = fgetc (FilePtr);

		   StillMatchingBytes = same_char ((char)ch, SearchString[FileOffset]);

		   if (StillMatchingBytes)
			FileOffset++;
	    }

	    if (StillMatchingBytes)
	    {
		  AddMatch (FilePosition);
		  count++;
		  ltoa (count, str, 10);
		  strcpy (str, CommaPad (str));
		  SetColourPair (palette.bright_white, palette.blue);
		  WriteString (46, 16, str);
	     }

	     FilePosition++;
	}
	SetColourPair (palette.black, palette.green);
	if (config_byte1 & MSK_BORDER_TYPE)
	   WriteMultiChar (30, 9, H_SINGLE, 20);
	else
	   WriteMultiChar (30, 9, H_DOUBLE, 20);
	write_blanks (30, 4, 46);
	restore_area (21, 14, 60, 19, InfoBuffer3);
	free ((char *)InfoBuffer3);
		
	ShowMatches (FilePtr, SearchString);          
	restore_area (2, 3, 78, 22, InfoBuffer);
	free ((char *)InfoBuffer);    
	ClearSrchLst ();
	fclose (FilePtr);
    }
}

void ShowMatches (FILE *FilePtr, char *SearchString)
{
    char *InfoBuffer;
    char *InfoBuffer2;
    struct SrchLst *ptr, *ptr2;
    int    quit;
    char ofs[67];
    char str1[67], str2[67];
    unsigned int ch;
    long    MCount;

    ptr  = SrchList;
    ptr2 = NULL;
    if (ptr == NULL)
    {
	InfoBuffer = (char *)malloc (PORTSIZE(25, 7, 56, 13));
	if (InfoBuffer != NULL)
	{
	    save_area(25, 7, 56, 13, InfoBuffer);
	    SetColourPair (palette.black, palette.cyan);
	    draw_window (25, 7, 55, 12);
	    WriteString (32, 9, "No Matches Found");
	    centre_push_button (25, 55, 11, "&Continue");
	    restore_area (25, 7, 56, 13, InfoBuffer);
	    free ((char *)InfoBuffer);
	}
    }
    else
    {
	quit = FALSE;
	MCount = 1L;
	while (! quit)
	{             
	     SetColourPair (palette.black, palette.green);                  
	     write_blanks (4, 7, 73);

	     WriteString (60, 7, "Match (");
	     ltoa (MCount, str1, 10);
	     SetColourPair (palette.red, palette.green);
	     WriteString (67, 7, str1);
	     SetColourPair (palette.black, palette.green);
	     WriteChar (67+strlen (str1), 7, '/');

	     ltoa (TotalMatches, str2, 10);
	     SetColourPair (palette.red, palette.green);
	     WriteString (68+strlen(str1), 7, str2);
	     SetColourPair (palette.black, palette.green);
	     WriteChar (68+strlen (str1)+strlen(str2), 7, ')');
				

	     WriteString (4, 7, "File Offset : ");
	     ltoa (ptr->SrchOffset, ofs, 10);
	     strcpy (ofs, CommaPad (ofs));
	     SetColourPair (palette.red, palette.green);
	     WriteString (18, 7, ofs);

	     ShowMatchContext (FilePtr, ptr->SrchOffset, SearchString);
			
	     ch = get_key ();
	     switch (ch)
	     {
		   case K_CSR_UP : if (ptr->PreviousMatch != NULL)
				   {
					    ptr = ptr->PreviousMatch;
					    MCount--;
				    }
				    break;
		   case K_CSR_DN : if (ptr->NextMatch != NULL)
				   {
					      ptr = ptr->NextMatch;
									       MCount++;
				   }
				   break;
		   case K_ESC    : quit = TRUE;
				   break;

		   case K_HOME : ptr = SrchList;
				 MCount = 1L;
				 break;

		   case K_END : ptr = SrchList;
				MCount = 1L;
				while (ptr->NextMatch != NULL)
				{
				       ptr = ptr->NextMatch;
				       MCount++;
				}
				break;
		   case K_F1  : InfoBuffer2 = (char *)malloc(PORTSIZE(15, 6, 66, 16));
				if (InfoBuffer2 != NULL)
				{
				      save_area (15, 6, 66, 17, InfoBuffer2);
				      SetColourPair (palette.black, palette.green);
				      draw_window (15, 6, 65, 16);
				      window_title (15, 65, 6, "File Search Help", palette.red, palette.green);
				      SetColourPair (palette.black, palette.green);
				      WriteString (18, 13, "CSR-UP/DN     - Step Through Text Matches.");
				      WriteString (18, 10, "HOME          - Jump To First Match.");
				      WriteString (18, 11, "END           - Jump To Last Match.");
				      WriteString (18, 12, "ESC           - Stop Viewing Matches.");
				      WriteString (18,  9, "F1            - View This Help Page.");
				      centre_push_button (15, 65, 15, "&Continue");
				      restore_area (15, 6, 66, 17, InfoBuffer2);
				      free ((char *)InfoBuffer2);
				}
				break;
		    default   : break;
	     }
	 }
    }
}

void ShowMatchContext (FILE *ptr, long position, char *SString)
{
    long i;
    int  xplace,
	 yplace;
    int  ch;

    SetColourPair (palette.black, palette.green);
    clear_area (4, 10, 76, 20);

    xplace = 4; yplace = 10; /* Initial Text Position */

    if (position >= 40L)
    {
	  rewind (ptr);
	
	  for (i = position - 40L; i < position; i++)
	  {
		 fseek (ptr, i, SEEK_SET);

		 ch = fgetc (ptr);
		 switch (ch)
		 {
		      case 9  : write_blanks (xplace, yplace, 1);
				xplace++;
				if (xplace > 76)
				{
				       xplace = 4;
				       yplace++;
				}                                     
				break;

		      case 10 : xplace = 4; yplace++;
				break;

		      case 13 : break;
		      default : WriteChar (xplace, yplace, (char)ch);
				xplace++;
				if (xplace > 76)
				{
				     xplace = 4;
				     yplace++;
				}
				break;
		 }
	   }

	   i = 0L;
	   while ((i < (long)strlen (SString)) && (yplace < 21))
	   {
		 SetColourPair (palette.red, palette.green);
		 WriteChar (xplace, yplace, (char)fgetc(ptr));
		 xplace++;
		 if (xplace > 76)
		 {
			 xplace = 4;
			 yplace++;
		 }
		 i++;
	   }
	   while ((yplace < 21) && (! feof (ptr)))
	   {
		 SetColourPair (palette.black, palette.green);
		 ch = fgetc (ptr);
		 switch (ch)
		 {
		      case  9 : write_blanks (xplace, yplace, 1);
				xplace++;
				if (xplace > 76)
				{
				      xplace = 4;
				      yplace++;
				}
				break;
		     case 10 : yplace++;
			       xplace = 4;
			       break;
		     case 13 : break;
		     default : WriteChar (xplace, yplace, (char)ch);
			       xplace++;
			       if (xplace > 76)
			       {
				      xplace = 4;
				      yplace++;
			       }
			       break;
		 }
	   }
    }                                   
    else
    {
	  rewind (ptr); 
     
	  for (i = 0L; i < position; i++)
	  {
		SetColourPair (palette.black, palette.green);
		ch = fgetc (ptr);
		switch (ch)
		{
		      case 9  : write_blanks (xplace, yplace, 1);
				xplace++;
				if (xplace > 76)
				{
				       xplace = 4;
				       yplace++;
				}                     
				break;
		     case 10 : yplace++;
			       xplace = 4;                   
			       break;
		     case 13 : break;
		     default : WriteChar (xplace, yplace, (char)ch);
			       xplace++;
			       if (xplace > 76)
			       {
				     xplace = 4;
				     yplace++;     
			       }
		 }
	  }
	  
	  for (i = 0L; i < (long)strlen (SString); i++)
	  {
		SetColourPair (palette.red, palette.green);
		WriteChar (xplace, yplace, (char)fgetc (ptr));
		xplace++;
		if (xplace > 76)
		{
		     xplace = 4;
		     yplace++;
		}
    } 
    while ((! feof (ptr)) && (yplace < 21))
    {     
	  SetColourPair (palette.black, palette.green);
	  ch = fgetc (ptr);
	  switch (ch)
	  {
	       case 9  :
			  write_blanks (xplace, yplace, 1);
			  xplace++;
			  if (xplace > 76)
			  {
				xplace = 4;
				yplace++;
			  }
			  break;
		 case 10 :yplace++;
			  xplace = 4;
			  break;
		 case 13 : break;
		 default : WriteChar (xplace, yplace, (char)ch);
			   xplace++;
			   if (xplace > 76)
			   {
				 xplace = 4;
				 yplace++;
			   }
			   break;
	  }
    }
    }                           
}       
			
		

/******************************************************/
/* Functions For Maintaining Matching Occurences List */
/******************************************************/

void InitSrchLst ()
{
    SrchList = NULL;
	 TotalMatches = 0L;     
}

void ClearSrchLst ()
{
    struct SrchLst *LastPtr,
		   *ThisPtr;

    ThisPtr = SrchList;
    LastPtr = NULL;
    while (ThisPtr != NULL)
    {
	LastPtr = ThisPtr;
	ThisPtr = ThisPtr->NextMatch;
	free ((struct SrchLst *)LastPtr);
    }
}

void AddMatch (long position)
{
    struct SrchLst *NewMatch,
		   *ThisPtr;

    NewMatch = (struct SrchLst *)malloc (sizeof (struct SrchLst));
    if (NewMatch != NULL)
    {    
		  TotalMatches++;       
	NewMatch->SrchOffset    = position;
	NewMatch->PreviousMatch = NULL;
	NewMatch->NextMatch     = NULL;
	if (SrchList == NULL)
	    SrchList = NewMatch;
	else
	{
	     ThisPtr = SrchList;
	     while (ThisPtr->NextMatch != NULL)
		 ThisPtr = ThisPtr->NextMatch;
	     NewMatch->PreviousMatch = ThisPtr;
	     ThisPtr->NextMatch      = NewMatch;
	}
    }
}

   

       
