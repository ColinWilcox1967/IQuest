#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <malloc.h>
#include <bios.h>
#include <dos.h>
#include "errlist.h"
#include "keys.h"
#include "config.h"
#include "iq.h"
#include "palette.h"
#include "windows.h"

#define WRAP_OFF       0
#define WRAP_ON        1

#define MAX_ENTRIES   56

extern unsigned char config_byte1;
extern char external_editor[];
extern int DriveList[];
extern char gen_filename[];
extern struct palette_str palette;
extern unsigned int dsk_status1, dsk_status2;
extern unsigned int AttachedDrives;

struct dir_s
{
    char DirEntry[13];
    struct dir_s *PreviousEntry;
    struct dir_s *NextEntry;
};

typedef struct dir_s *dir_p;

extern void centre_push_button (int x1, int x2, int y, char msg[]);
extern void  TextViewer ( char name [] );
extern void  TextSearch (char name[], int mode);
extern void  ScrollUp (unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern void  ScrollDown (unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
extern dir_p FindEntryN (unsigned int);
extern void  WriteMultiChar (int, int, char, int);
extern void  WriteString ( int,  int, char *);
extern void show_error ( int errnum);

extern       unsigned int get_key (void);
extern void  InitDirList (void);
extern void  ClearDirList (void);
extern int   BuildDirList (void);
extern void  ShowDir (unsigned int, unsigned int);
extern void  clear_area (int, int,  int, int);
extern char  *PaddedFilename (char *);
extern dir_p DirList;
extern void  save_area ( int,  int,  int,  int, char *);
extern void  restore_area ( int, int, int, int, char *);
extern void  draw_window (int,  int, int,  int);
extern void  ShowDirList (unsigned int, unsigned int, unsigned int);
extern void  DrawFileViewer (char *);
extern void  hide_cursor (void);
extern void  UseViewer (char name[], long size);
extern void  ShowDir (unsigned int, unsigned int);
extern unsigned int CheckDriveStatus (unsigned int);
extern void  ShowDrive (int, unsigned int);
extern void  ReportDriveStatus (unsigned int);
extern int   DiskInDrive (unsigned int);
extern void  SetDriveList ( void );
extern void  window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void  centre_push_button (int x1, int x2, int y, char msg[]);

extern int ActiveDrive;
extern int LastDrive;

FILE *vwr_file;

unsigned int DirEntryCount;
unsigned int TotalDrives;
unsigned int InitialDrive;
char         InitialDir[67];

void         GetFileFromDirectory ( void );   
char         *DosVersion ( void );
char         *FreeMemory ( void );
unsigned int GetDirSize ( void );
void         ShowEmptyDirectory ( void );
void         ShowCurrentPath ( void );
void         RescanDrive ( void );
void         ShowRebuildMsg ( void );

char *trimmed ( char *str, int n)
{
   char s[80];

   if (strlen (str) <= n)
      return (str);
   else
   {
      strcpy (s, strrev (str));
      s[strlen(s) - n - 1 ] = (char)0;
      return (strrev (s));
   }
}

unsigned int GetDirSize()
{
   unsigned int count;
   dir_p        ThisEntry;

   ThisEntry = DirList;
   count     = 0;
   while ( ThisEntry != NULL )
   {
       count++;
       ThisEntry = ThisEntry->NextEntry;
   }
   return ( count );
}

void ShowEmptyDirectory ()
{
    if (GetDirSize() == 0) /* Entry exists so clear any old message window */
    {
	SetColourPair (palette.bright_white, palette.blue);
	draw_window(28, 8, 53, 10);
	SetColourPair (palette.yellow, palette.blue);
	WriteString (32, 9, "Directory Is Empty");
    }
    SetColourPair (palette.black, palette.bright_white);
}

void ShowEntry (unsigned int index, unsigned int start, unsigned int state)
{
    char padded[13];
    dir_p ptr;

    if ( state)
	SetColourPair (palette.red, palette.bright_white);
    else
	SetColourPair (palette.black, palette.bright_white);

    ptr = (struct dir_s *)malloc(sizeof (struct dir_s));
    ptr = FindEntryN (index);

    if (ptr->DirEntry[0] == '<')
	strcpy (padded, ptr->DirEntry);
    else
	strcpy (padded, PaddedFilename (ptr->DirEntry));

    switch ((index - start) % 4)
    {
	 case 0 : WriteString ( 6, 8 + ((index - start) / 4), padded);
		  break;
	 case 1 : WriteString (26, 8 + ((index - start) / 4), padded);
		  break;
	 case 2 : WriteString (46, 8 + ((index - start) / 4), padded);
		  break;
	 case 3 : WriteString (63, 8 + ((index - start) / 4), padded);
		  break;
     }
     free ((struct dir_s *)ptr);
}

char *FreeMemory ()
{
    char memory[256];

    itoa (_memavl (), memory, 10);
    return (memory);
}

char *DosVersion ()
{
    char version[5];

    union REGS regs;

    regs.h.ah = 0x30;
    regs.h.al = 0;
    intdos (&regs, &regs);
    version[0] = (char)(regs.h.al + 0x30);
    version[1] = '.';
    if ( regs.h.ah >= 10)
    {
	version[2] = (char)(0x30+(regs.h.ah / 10));
	version[3] = (char)(0x30+(regs.h.ah % 10));
	version[4] = (char)0;

    }
    else
    {    
	version[2] = (char)(regs.h.ah + 0x30);
	version[3] = (char)0;
    }

    return (version);
}

char *CreationTime (char *fn)
{
    struct find_t c_file;
    char          cr_time[67], str[10];
    int           x;
    int           period;

    _dos_findfirst (fn, FILE_ATTR_LIST, &c_file);
    
    x = c_file.wr_time; x = x >> 11; x &= 31;
    if (x > 12)
    {
	x-=12;
	period = 1;
    }
    else
	period = 0;
    itoa (x, str, 10);
    if (strlen (str) == 1)
    {
	str[1] = str[0]; str[0]= ' '; str[2] = (char)0;
    }
    strcpy (cr_time, str);
    strcat (cr_time, ":");
    x = c_file.wr_time; x = x >> 5; x &= 63;
    itoa (x, str, 10);
    if (strlen (str) == 1)
    {
	str[1] = str[0]; str[0]='0'; str[2]=(char)0;
    }
    strcat (cr_time, str);
    if (period == 1) strcat (cr_time, "pm"); else strcat (cr_time, "am");
    
    return (cr_time);
}

char *CreationDate (char *fn)
{
    struct find_t c_file;
    char          cr_date[67], str[10];
    int           x;

    _dos_findfirst (fn, FILE_ATTR_LIST, &c_file);
    
    x = c_file.wr_date & 31;
    itoa (x, str, 10);
    switch (x)
    {
	case 1 :
		  case 21:
		  case 31: strcat (str, "st ");
					  break;
		  case  2:
		  case 22: strcat (str, "nd "); 
					  break;
		  case  3:
		  case 23: strcat (str, "rd ");
					  break;
		  default: strcat (str, "th ");
					  break;
    }
    strcpy (cr_date, str);
    x = c_file.wr_date; x = x >> 5; x &= 15;
    switch (x)
    {
	case 1 : strcat (cr_date, "January ");
		 break;
	case 2 : strcat (cr_date, "February ");
		 break;
	case 3 : strcat (cr_date, "March ");
		 break;
	case 4 : strcat (cr_date, "April ");
		 break;
	case 5 : strcat (cr_date, "May ");
		 break;
	case 6 : strcat (cr_date, "June ");
		 break;
	case 7 : strcat (cr_date, "July ");
		 break;
	case 8 : strcat (cr_date, "August ");
		 break;
	case 9 : strcat (cr_date, "September ");
		 break;
	case 10: strcat (cr_date, "October ");
		 break;
	case 11 : strcat (cr_date, "November ");
		  break;
	case 12 : strcat (cr_date, "December ");
		  break;
    } 
    x = c_file.wr_date; x = x >> 9; x &= 127; x += 1980;
    itoa (x, str, 10); strcat (cr_date, str);
    return (cr_date);
}

char *CommaPad (char *s)
{
    char str[255];
    int count, index, i;

    count = 0;
    index = 0;
    str[0] = (char)0;
    for (i= (int)strlen(s)-1; i >= 0; i--)
    {
	count++;
	str[index++] = s[i];
 
	if (count == 3)
	{
	      if (i > 0)
		  str[index++]=',';
	      count = 0;
	}
    }
    str[index] = (char)0;
    return (strrev (str));
}
	
    
char *SizeOfFile (char *fn)
{
    char          fsize[67];
    struct find_t c_file;

    _dos_findfirst (fn, FILE_ATTR_LIST, &c_file);
    ltoa (c_file.size, fsize, 10);
    return (fsize);
}
    
void ShowRebuildMsg ()
{
    SetColourPair (palette.bright_white, palette.red);
    draw_window (27, 9, 53, 12);
    SetColourPair (palette.yellow, palette.red);
    WriteString ( 30, 10, "Rescanning Directory");
    WriteString ( 31, 11, " ** PLEASE WAIT **");
}

void ShowEntryStats (char *fn)
{
    struct find_t c_file;

    char cr_date[67],
	 cr_time[67],
	 file_size[67],
	 version[5],
	 dir[67],
	 d[67];
    int  i;
    long TotalFiles,
	 TotalSize;


    strcpy (cr_date, CreationDate (fn));
    strcpy (cr_time, CreationTime (fn));
    strcpy (version, DosVersion ());
    strcpy (file_size, CommaPad (SizeOfFile (fn)));

    if (fn[0] != '<')
    {
	SetColourPair (palette.black, palette.green);
	WriteString (7, 5, "File Name : ");
	WriteString (7, 6, "File Size : ");
	WriteString (19+strlen (file_size), 6, " Bytes");
	WriteString (39, 5, "(MS-DOS Version : ");
	WriteString (57+strlen (version), 5, ")");
	WriteString (7, 8, "Created On ");
	WriteString (18 + strlen (cr_date)+2, 8, "At ");
	SetColourPair (palette.red, palette.green);
	WriteString (19, 5, fn);
	WriteString ( 19, 6, file_size);
	WriteString ( 57, 5, version);
	WriteString ( 18, 8, cr_date);
	WriteString (18 + strlen (cr_date)+5, 8, cr_time);
    }
    else
    {
	SetColourPair (palette.black, palette.green);
	WriteString (7, 5, "Directory Name : ");
	SetColourPair (palette.red, palette.green);
	if (strcmp (fn, "<**PARENT**>") == 0)
	{
	     WriteString (24, 5, "..");
	     strcpy (d, "..");
	}
	else
	{
	     i=1;
	     while(fn[i] != '>')
	     {
		  d[i-1] = fn[i];
		  i++;
	     }
	     d[i-1] = (char)0;
	     WriteString (24, 5, d);
	 }
	 SetColourPair (palette.black, palette.green);
	 WriteString (43, 5, "(MS-DOS Version : ");
	 WriteString (61+strlen (version), 5, ")");
	 SetColourPair (palette.red, palette.green);
	 WriteString ( 61, 5, version);
	 getcwd (dir, 66);

	 chdir (d);
	 TotalFiles = TotalSize = 0L;
	 if (_dos_findfirst ("*.*", FILE_ATTR_LIST, &c_file) == 0)
	 {
	      TotalFiles++;
	      TotalSize += c_file.size;
	      while (_dos_findnext (&c_file) == 0)
	      {
		      TotalFiles++;
		      TotalSize += c_file.size;
	      }
       }
       chdir (dir);

       ltoa (TotalFiles, d, 10);
       strcpy (d, CommaPad (d));

       SetColourPair (palette.black, palette.green);
       WriteString (7, 7, "Total Files = ");
       SetColourPair (palette.red, palette.green);
       WriteString (21, 7, d);
      
       ltoa (TotalSize, d, 10);
       strcpy (d, CommaPad (d));

       SetColourPair (palette.black, palette.green);
       WriteString (7, 8, "Total Size  = ");
       WriteString (21+strlen (d), 8, " Bytes");
       SetColourPair (palette.red, palette.green);
       WriteString ( 21, 8, d);
    }                      
    centre_push_button (1, 79, 10, "&Continue");
}
    

void ShowCurrentPath ()
{
    char d[255];

    SetColourPair (palette.black, palette.bright_white);
    getcwd (d, 254);
    WriteString (12, 6, d);
    WriteMultiChar (12+strlen (d), 6, ' ', 67-12-strlen (d));    
}

void RescanDrive ()
{
    SetColourPair (palette.black, palette.white);
    ClearDirList();
    InitDirList();
    if (BuildDirList() != 0)
       show_error (ERR_NO_FILE_MEMORY);
    ShowCurrentPath();
}


void SelectFromDir (unsigned int);

void SelectFromDir (unsigned int start)
{
    int          environment;
    unsigned int ch,
		 EntryCount,
		 FirstEntry,
		 LastEntry;
    int          AbortAction,
		 ChangeMode,   
		 ChoiceMade,
		 i;
    char         FullPath[255],
		 OldDir[255],
		 d[67];
    char         *InfoBuffer;
    dir_p        ThisEntry;

    hide_cursor ();
    getcwd (OldDir, 254);
    ChoiceMade = AbortAction = FALSE;
    
    _dos_getdrive (&ActiveDrive);
    ActiveDrive--;

    DirEntryCount = GetDirSize();
    FirstEntry    = start;
    EntryCount    = 0;
    ThisEntry     = DirList;
    LastEntry     = min ( start + MAX_ENTRIES - 1, DirEntryCount );
    environment = 2;

    while (! (ChoiceMade || AbortAction))
    {
	ShowCurrentPath ();   
	ShowDrive (ActiveDrive, TRUE);
			
	if (environment == 1)
	{
	     AbortAction = 0;
	     ChangeMode  = 0;
	     SetColourPair (palette.red, palette.bright_white);
	     WriteString (3, 2, "Drives");

	     while (! AbortAction && ! ChangeMode)
	     {
		  ch = get_key ();
		  switch (ch)
		  {
		       case K_F1     : InfoBuffer = (char *)malloc (PORTSIZE(15, 1, 66, 10));
				       if (InfoBuffer != NULL)
				       {      
					     save_area (15, 1, 66, 10, InfoBuffer);
					     SetColourPair (palette.black, palette.green);
					     draw_window (15, 1, 65, 9);
					     window_title ( 15, 65, 1, "Drive Selection Help", palette.red, palette.green);
					     SetColourPair (palette.black, palette.green);
					     WriteString (17,  4, "CSR-LT/RT    - Move Through Drive List.");
					     WriteString (17,  5, "ESC          - Abort Drive Selection.");
					     WriteString (17, 6, "F1           - Display This Help Screen.");
					     WriteString (17, 7, "ALT-F/ENTER  - Move to file selection area.");
					     centre_push_button (15, 65, 9, "&Continue");
					     restore_area (15, 1, 66, 10, InfoBuffer);              
					     free ((char *)InfoBuffer);
					 }
					 break;      
			 case K_CSR_LT : 
					 ShowDrive (ActiveDrive, FALSE);
					 if (ActiveDrive == 0)
					       ActiveDrive = LastDrive;
					 else
					 {
					      ActiveDrive--;
					      while ((DriveList[ActiveDrive] == 0) &&
						     (ActiveDrive >= 0)
						    )   
						      ActiveDrive--;
					 }
					 ShowDrive (ActiveDrive, TRUE);
					 ShowCurrentPath();
					 if (DiskInDrive (ActiveDrive))
					 {
					     InfoBuffer = (char *)malloc(PORTSIZE(23, 9, 58, 14));
					     if (InfoBuffer != NULL)
					     {
						 save_area (23,9,58,14,InfoBuffer);
						 ShowRebuildMsg ();
					     }
					     _dos_setdrive(ActiveDrive+1, &TotalDrives);
					     RescanDrive ();
					     if (InfoBuffer != NULL)
					     {
						 restore_area (23,9,58,14,InfoBuffer);
						 free((char *)InfoBuffer);
					     }
					     ShowDirList (0, 255, 0);
					     ShowEmptyDirectory();
					 }
					 else
					     ReportDriveStatus (ActiveDrive);
					 break;
			 case K_CSR_RT : 
					 ShowDrive (ActiveDrive, FALSE);
					 ShowCurrentPath();
					 if (ActiveDrive == LastDrive)
					     ActiveDrive = 0;
					 else
					 {
					     ActiveDrive++;
					     while ( (DriveList[ActiveDrive] == 0) &&
						    (ActiveDrive <= LastDrive)
						   )
						ActiveDrive++;
					 }
					 ShowDrive (ActiveDrive, TRUE);
					 if (DiskInDrive(ActiveDrive))
					 {    
					     InfoBuffer = (char *)malloc(PORTSIZE(23,9,58,14));
					     if (InfoBuffer != NULL)
					     {
						 save_area (23,9,58,14,InfoBuffer);
						 ShowRebuildMsg();
					     }
					     _dos_setdrive (ActiveDrive+1, &TotalDrives);
					     RescanDrive ();
					     DirEntryCount = GetDirSize();
					     if (InfoBuffer != NULL)
					     {
						 restore_area (23,9,58,14,InfoBuffer);
						 free ((char *)InfoBuffer);
					     }
					     ShowDirList (0, 255, 0);
					     ShowEmptyDirectory();
					 }
					 else
					     ReportDriveStatus(ActiveDrive);
					 break;
			 case K_ESC    : AbortAction = TRUE;
					 FullPath[0] = (char)0;
					 break;
			 case K_ALT_F :   
			 case K_CR    : if ((DiskInDrive (ActiveDrive)) &&
					    ( GetDirSize() > 0)
					   )
					{
					      environment = 2;
					      SetColourPair (palette.black, palette.bright_white);
					      WriteString ( 4, 2, "rives");
					      SetColourPair (palette.red, palette.bright_white);
					      WriteString ( 3, 6, "Files In");
					      WriteChar (3, 2, 'D');
					      ChangeMode = TRUE;
					      _dos_setdrive (ActiveDrive+1, &TotalDrives);
					      RescanDrive ();
					      DirEntryCount = GetDirSize();
					      ThisEntry = DirList;
					      ShowDirList(0, 255, 0);
					      EntryCount=0;
					  }
					  else
					      ReportDriveStatus(ActiveDrive);
					  break;      
			 default  : break;
		  }
	     }
	}
	else
	{     
	     AbortAction = FALSE;
	     ChangeMode = FALSE;
	     ChoiceMade = FALSE;
	     WriteString (3, 6, "Files In");
	     while ((! AbortAction) && (! ChangeMode) && (! ChoiceMade))
	     {
		  ShowEntry (FirstEntry + EntryCount, FirstEntry, TRUE);
		  ch = get_key ();
		  if ((ch != K_ALT_I) && (DiskInDrive (ActiveDrive)))
		       ShowEntry (FirstEntry + EntryCount, FirstEntry, FALSE);
		  switch (ch)
		  {
			   case K_ALT_D  : if (DiskInDrive (ActiveDrive))
					   {     
					       SetColourPair (palette.black, palette.bright_white);
					       WriteString (4, 6, "iles In");
					       WriteString (3,2 , "Drives");
					       SetColourPair (palette.red, palette.bright_white);
					       WriteChar (3, 6, 'F');
					       environment = 1;
					       ChangeMode = TRUE;
					       InfoBuffer = (char *)malloc(PORTSIZE(23, 9, 58, 14));
					       if (InfoBuffer != NULL)
					       {
						   save_area (23,9,58,14, InfoBuffer);
						   ShowRebuildMsg();
					       }
					       _dos_setdrive (ActiveDrive+1, &TotalDrives);
					       RescanDrive ();
					       DirEntryCount = GetDirSize();
					       if (InfoBuffer != NULL)
					       {
						   restore_area (23,9,58,14, InfoBuffer);
						   free ((char *)InfoBuffer);
					       }
					       ShowDirList (0, 255, 0);
					   }
					   else
					       ReportDriveStatus(ActiveDrive);
					   break;
			   case K_ALT_F  : if (DiskInDrive (ActiveDrive))
					   {
						if (ThisEntry->DirEntry[0] == '<')
						{
						       InfoBuffer = (char *)malloc (PORTSIZE(24, 9, 55, 14));
						       if (InfoBuffer != NULL)
						       {
							    save_area (24, 9, 55, 14, InfoBuffer);
							    SetColourPair (palette.black, palette.cyan);
							    draw_window (24, 9, 54, 13);
							    WriteString (27, 10, "You Can Only Search Files");
							    centre_push_button ( 24, 54, 12, "&Continue");
							    restore_area (24, 9, 55, 14, InfoBuffer);
							    free ((char *)InfoBuffer);
							}
						}
						else
						{
							TextSearch (ThisEntry->DirEntry, config_byte1 & MSK_FILE_VIEWER);
						}                       
					   }
					   else
					       ReportDriveStatus (ActiveDrive);
					   break;
	
			   case K_F1    : InfoBuffer = (char *)malloc (PORTSIZE(15, 3, 66, 19));
					  if (InfoBuffer != NULL)
					  {
						 save_area (15, 3, 66, 19, InfoBuffer);
						 SetColourPair (palette.black, palette.green);
						 draw_window (15, 3, 65, 18);
						 window_title ( 15, 65, 3, "File Selection Help", palette.red, palette.green);
						 SetColourPair (palette.black, palette.green);
						 WriteString (17,  6, "Cursor Keys      - Used For Highlighting" );
						 WriteString (17,  7, "                   Files Or Directories.");
						 WriteString (17,  8, "HOME             - Move To Top Of Directory.");
						 WriteString (17,  9, "ESC              - Abort File Selection.");
						 WriteString (17, 10, "ENTER            - Choose Highlighted Entry.");
						 WriteString (17, 11, "F1               - Display This Help Page.");
						 WriteString (17, 12, "ALT-I            - Show Details Of Highlighted");
						 WriteString (17, 13, "                   Entry."); 
						 WriteString (17, 14, "ALT-F            - Search Current File.");
						 WriteString (17, 15, "ALT-D            - Change Active Drive.");
						 centre_push_button (15, 65, 17, "&Continue");
						 restore_area (15, 3, 66, 19, InfoBuffer);
						 free ((char *)InfoBuffer);
					   }
					   break;
			    case K_HOME   : if (DiskInDrive (ActiveDrive))
					    {
						
						FirstEntry = 0;
						LastEntry = ( MAX_ENTRIES - 1, DirEntryCount);
						EntryCount = 0;
						ThisEntry = DirList;
						ShowDir (FirstEntry, LastEntry);
					    }
					    else
						ReportDriveStatus (ActiveDrive);
					    break;  
			    case K_ALT_I  : if (DiskInDrive (ActiveDrive))
					    {
						 InfoBuffer = (char *)malloc (PORTSIZE (1, 3, 80, 15));
						 if (InfoBuffer != NULL)
						 {
						      save_area (5, 3, 76, 12, InfoBuffer);
						      SetColourPair (palette.black, palette.green);
						      draw_window (5, 3, 75, 11);
						      if (ThisEntry->DirEntry[0] == '<')
							   ShowEntryStats (ThisEntry->DirEntry);
						      else
							   ShowEntryStats (ThisEntry->DirEntry);
						      restore_area (5, 3, 76, 12, InfoBuffer);
						      free ((char *)InfoBuffer);      
						  }       
					      }
					      else
						  ReportDriveStatus (ActiveDrive);
					      break;  

			     case  K_ESC    : AbortAction = TRUE;
					      FullPath[0] = (char)0;
					      ClearDirList ();
					      InitDirList ();
					      break;
			     case  K_CR     : if (DiskInDrive (ActiveDrive))
					      {
						  if (ThisEntry->DirEntry[0] == '<')
						  {
							if (strcmp (ThisEntry->DirEntry, "<**PARENT**>") == 0)
							{
							       ClearDirList ();
							       InitDirList ();
							       chdir ("..");
							       InfoBuffer = (char *)malloc (PORTSIZE(23, 9, 58, 14));
							       if (InfoBuffer != NULL)
							       {
								   save_area (23, 9, 58, 14, InfoBuffer);
								   ShowRebuildMsg ();
							       }
							       if (BuildDirList () != 0)
								  show_error (ERR_NO_FILE_MEMORY);
							       DirEntryCount = GetDirSize();
							       if (InfoBuffer != NULL)
							       {
								   restore_area(23, 9, 58, 14, InfoBuffer);
								   free ((char *)InfoBuffer);
							       }
									     
							       ShowDir (0, MAX_ENTRIES-1);
							       SetColourPair (palette.red, palette.bright_white);
							       WriteString (3, 6, "Files In");
							       ShowDrive (ActiveDrive, TRUE);
							       EntryCount = 0;
							       FirstEntry = 0;
							       ThisEntry  = DirList;
							       LastEntry  = min (MAX_ENTRIES - 1, DirEntryCount);
							  }
							  else
							  {
							       i = 1;
							       while (ThisEntry->DirEntry[i] != '>')
							       {
								      d[i - 1] = ThisEntry->DirEntry[i];
								      i++;
							       }
							       d[i-1]=(char)0;
							       chdir (d);
							       ClearDirList ();
							       InitDirList ();
							       InfoBuffer = (char *)malloc (PORTSIZE(23, 9, 58, 14));
							       if (InfoBuffer != NULL)
							       {
								   save_area (23, 9, 58, 14, InfoBuffer);
								   ShowRebuildMsg ();
							       }
							       if (BuildDirList () != 0)
								  show_error (ERR_NO_FILE_MEMORY);
							       DirEntryCount = GetDirSize();
							       if (InfoBuffer != NULL)
							       {
								   restore_area (23, 9, 58, 14, InfoBuffer);
								   free ((char *)InfoBuffer);
							       }
							       ShowDir (0, MAX_ENTRIES - 1);
							       SetColourPair (palette.red, palette.bright_white);
							       WriteString (3, 6, "Files In");
							       ShowDrive (ActiveDrive, TRUE);
							       EntryCount = 0;
							       ThisEntry  = DirList;
							       FirstEntry = 0;
							       LastEntry  = min ( MAX_ENTRIES - 1, DirEntryCount );
							 }
						  }                       
						  else
						  {
							getcwd (FullPath, 254);
							if ( FullPath[strlen (FullPath)-1] != '\\')
								strcat (FullPath, "\\");
							strcat (FullPath, ThisEntry->DirEntry);
							ChoiceMade = TRUE;
						   }
					      }
					      else
						  ReportDriveStatus (ActiveDrive);
					       break;
	
			       case K_CSR_LT : 
					       if ( DiskInDrive (ActiveDrive) )
					       {
						    if ( FirstEntry + EntryCount > 0)
						    {
							if ( EntryCount == 0 )
							{
								EntryCount += 3;
								FirstEntry -= 4;
								ThisEntry  = FindEntryN ( FirstEntry + EntryCount);
								LastEntry  = min ( FirstEntry + MAX_ENTRIES - 1, DirEntryCount );
								ShowDir ( FirstEntry, LastEntry );
							}               
							else
							{
							       ThisEntry = ThisEntry->PreviousEntry;
							       EntryCount--;
							}
						    }
						}
						else
						    ReportDriveStatus( ActiveDrive );
						break;

			       case K_CSR_RT  : 
						if ( DiskInDrive (ActiveDrive) )
						{
						    if ( FirstEntry + EntryCount < DirEntryCount - 1)
						    {
							if (FirstEntry + EntryCount == LastEntry)
							{
							      if ( LastEntry + 4 < DirEntryCount )
							      {
								  FirstEntry += 4;
								  EntryCount -=3;
								  ThisEntry = FindEntryN ( FirstEntry + EntryCount);
								  LastEntry = min ( FirstEntry + MAX_ENTRIES - 1, DirEntryCount );
									   
								  ShowDir (FirstEntry, LastEntry);
							      }  
							}
							else          
							{
							     
							     EntryCount++;
							     ThisEntry  = ThisEntry->NextEntry;
							}
						    }
						}
						else
						    ReportDriveStatus (ActiveDrive);
						break;

				case K_CSR_UP  : if (DiskInDrive (ActiveDrive))
						 {
						     if (EntryCount < 4)      /* on top line ? */
						     {
							 if ( FirstEntry > 3)  /* another line above ? */
							 {
							     FirstEntry-=4;
							     ThisEntry = FindEntryN (FirstEntry+EntryCount);
							     LastEntry = min ( FirstEntry + MAX_ENTRIES - 1, DirEntryCount );
							     ShowDir (FirstEntry, LastEntry);
							 }                   
						     }    
						     else
						     {
							 EntryCount -= 4;
							 ThisEntry = FindEntryN (FirstEntry+EntryCount);
						     }
						 }
						 else
						     ReportDriveStatus (ActiveDrive);
						 break;

				case K_CSR_DN  : if (DiskInDrive (ActiveDrive))
						 {
						     if (LastEntry - (FirstEntry + EntryCount)  < 4) /* On last line */
						     {
							 if (LastEntry + 4 < DirEntryCount) /* Another line below ? */
							 {
								FirstEntry +=4;
								ThisEntry = FindEntryN ( FirstEntry+EntryCount );
								LastEntry = min (FirstEntry + MAX_ENTRIES - 1, DirEntryCount );
								ShowDir (FirstEntry, LastEntry);
							  }
						      }
						      else
						      {
							    if ( EntryCount + 4 < DirEntryCount)
							    {
								    EntryCount += 4;
								    ThisEntry = FindEntryN (FirstEntry+EntryCount);
							    }         
						      }
						  }
						  else
						      ReportDriveStatus (ActiveDrive);
						  break;
				 default        : break;
		  }
	     }
	}
    }
    chdir (OldDir);
    if (! AbortAction)
	strcpy ( gen_filename, FullPath);
    else
	gen_filename[0] = (char)0;
}

void GetFileFromDirectory ()
{
    char *dos_dir_buff;

    /* Store Location Details for later */

    _dos_getdrive (&InitialDrive);
    getcwd (InitialDir, 66);

    dos_dir_buff = (char *)malloc(PORTSIZE(1,1,80,25));
    if (dos_dir_buff == NULL)
    {
	gen_filename[0] = (char)0;
	return;
    }
    save_area (1,1,80,25, dos_dir_buff);
    InitDirList();
    ClearDirList();
    if (BuildDirList() != 0)
       show_error (ERR_NO_FILE_MEMORY);
    DirEntryCount = GetDirSize();
    SetDriveList();
    hide_cursor();
    ShowDir(0, 255);
    SelectFromDir (0);

    restore_area (1,1,80,25, dos_dir_buff);
    free ((char *)dos_dir_buff);
    
    /* Restore Back To Initial Place */

    _dos_setdrive (InitialDrive, &TotalDrives);
    chdir (InitialDir);

}

