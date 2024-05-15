#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <bios.h>
#include <memory.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>

#include "errlist.h"
#include "keys.h"
#include "palette.h"
#include "windows.h"
#include "iq.h"
#include "doprint.h"

#define PAGE_HEADER1     "IQuest v1.1 (c) IQuest Software 1995,1996                    Page %3u"  
#define PAGE_HEADER2     "----------------------------------------------------------------------"

#define PRT_TIMEOUT      1
#define PRT_DONE         144
#define PRT_BAD_PRINTER  48
#define PRT_BAD_WRITE    49


unsigned int printer_status;

FILE *file_ptr;
int page;

struct printer_str
{
    char          pr_description[MAX_PR_DESC_SIZE+1];
    unsigned char lf_byte,
                  ff_byte, 
                  cr_byte;
    char          pr_init_string[MAX_PR_INIT_SIZE+1];
    unsigned char line_length_byte,
                  page_length_byte,
                  blank_lines_byte;
};

struct printer_str basic_pr  = { "Standard Printer", (char)0x0A, (char)0x0C, (char)0x0D,
                                 "", (char)0x50, (char)0x42, (char)0x00
                               };
extern struct printer_str active_pr;

struct print_time
{
    int           mark_secret;
    int           eol_terminator;
    int           throw_page;
    int           using;
    unsigned char printer_port;
    int           header_mode;
    int           suppress_mode;
}; 

struct print_time old_options, 
                  print_options;

extern struct palette_str palette;

extern void window_title ( int x1, int x2, int y, char msg[], short fg, short bk);
extern unsigned int draw_button ( int x, int y, int width, char msg[], int state);
extern void simulate_push (int x, int y, char msg[], int width);
extern void push_wait ( void );
extern void write_blanks (int, int, int);
extern int abandon_option_changes( void );
extern unsigned int get_key ( void );
extern void show_error (int errnum);
extern int file_exists (char filename[]);
extern void centre_push_button ( int x1, int x2, int y, char msg[]);
extern void save_pr_file (char name[]);
extern void load_pr_file (char name[]);
extern void select_file_to_print ( void );

struct find_t c_file;
unsigned int lines_printed,
             chars_printed;

void         create_default_printer_file ( void );
void         ReportPrinterError ( void );
void         create_default_printer_file ( void );
unsigned int WriteEOL ( void );
void         do_print ( void );
void         show_printer_settings ( void );   
void         set_init_print_time_options ( void );
void         show_print_time_options ( void );   
void         throw_page ( void );
void         print_file ( char filename[] );
void         print_header (int page_no);
unsigned int InitPrinter ( int port);
unsigned int SendPrinterS (int port, char text[]);

void create_default_printer_file ()
{
   memcpy (&active_pr.pr_description[0], &basic_pr.pr_description[0], sizeof (struct printer_str));
   save_pr_file (DEFAULT_PR_FILE);
}

void print_header (int this_page )
{
    char str[256];

    if ( print_options.header_mode == PRINTER_NO_HEADER )
        return; /* No header needs to be printed */

    if (( print_options.header_mode == PRINTER_FIRST_PAGE) && ( page > 1))
        return; /* Not the first page */


    if ( active_pr.line_length_byte >= 20)
    {
        chars_printed = 0;
        sprintf ( str, PAGE_HEADER1, this_page);
        str[active_pr.line_length_byte] = (char)0;
        printer_status = SendPrinterS( print_options.printer_port, str);
        if ( printer_status == PRT_DONE)
        {
            printer_status = WriteEOL();
            if ( printer_status ==  PRT_DONE)
            {
                chars_printed = 0;
                strcpy ( str, PAGE_HEADER2);
                str[active_pr.line_length_byte] = (char)0;
                printer_status = SendPrinterS (print_options.printer_port, str);
                if ( printer_status == PRT_DONE)
                {
                    /* Blank Line After Page Heading */
                    printer_status = WriteEOL();
                    if ( printer_status == PRT_DONE)
                    {
                        printer_status = WriteEOL();
                        if ( printer_status != PRT_DONE)
                            ReportPrinterError();
                        else
                            lines_printed = 3;
                    }
                    else 
                        ReportPrinterError();
                }
                else
                    ReportPrinterError();
            }
            else
                ReportPrinterError();
        }
        else
           ReportPrinterError();
    }
}

void print_file (char name[])
{
    char *print_buff;
    char str[20];
    int byte;
    char one_line[255];

    int one_line_idx, idx2;

    print_buff = (char *)malloc(PORTSIZE (25, 10, 56, 16));
    if ( print_buff == NULL)
       return;
    save_area ( 25, 10, 56, 16, print_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window ( 25, 10, 55, 15);
    SetColourPair ( palette.red, palette.blue);
    SetColourPair (palette.yellow, palette.blue);
    WriteString ( 30, 11, "Initialising Printer");
    sprintf (str, "On LPT%d", print_options.printer_port+1);
    WriteString ( 37, 12, str);
    centre_push_button (25, 55, 14, "&Proceed");

    /* Establish File OK */

    if (! file_exists (name))
    {  
        show_error ( ERR_NO_FILE );
        restore_area (25, 10, 56, 16, print_buff);
        free ((char *)print_buff);
        return;
    }

    file_ptr = fopen (name, "rb");
    printer_status = InitPrinter (print_options.printer_port);
    if ( printer_status != PRT_DONE)
       ReportPrinterError();
    else
    {
        printer_status = SendPrinterS (print_options.printer_port, active_pr.pr_init_string);
        if (printer_status == PRT_DONE)
        {
           page          = 1;
           lines_printed = 0;
           print_header (page);
           
           while (!feof (file_ptr))   
           {
              chars_printed  = 0;
              write_blanks (36, 12, 10);
              sprintf (str, "Page  %-d", page);
              SetColourPair (palette.yellow, palette.blue);
              WriteString ( 36, 12, str);
              if ( lines_printed >= active_pr.page_length_byte)    
              {
                  throw_page ();
                  lines_printed = 0; 
                  chars_printed = 0;
                  print_header(page);
              }
            
              /* All the hard work's done here */
               
               one_line_idx = 0;
               byte = fgetc (file_ptr);

               /* Check for secret mode */

               if (byte & SECRET_BIT_MASK) /* Bit 7 Set in first byte ? */
               {
                  if (print_options.mark_secret)
                        printer_status = SendPrinterS ((int)print_options.printer_port, "** SECRET **");
                  if (printer_status != PRT_DONE)
                     ReportPrinterError ();
                  else
                  {
                    lines_printed++;
                    WriteEOL();
                  }
               }
               
               while ( (! feof (file_ptr)) &&
                       (one_line_idx < 255) &&
                       (byte != (char)13) &&
                       ( printer_status == PRT_DONE)
                     )
               {
                  switch (print_options.suppress_mode)
                  {
                     case NO_SUPPRESSION     : 
                                               one_line[one_line_idx++] = (char)byte;
                                               break;
                     case SUPPRESS_NON_ASCII : 
                                                if (byte >= 32)
                                                      one_line[one_line_idx++] = (char)byte;
                                               break;
                     case SPACE_REPLACEMENT  : 
                                                  if (byte >= 32)
                                                      one_line[one_line_idx++] = (char)byte;
                                                   else
                                                      one_line[one_line_idx++] = ' ';
                                               break;
                  }
                  byte = fgetc (file_ptr);
               }
               one_line[one_line_idx] = (char)0;

               printer_status = SendPrinterS((int)print_options.printer_port, one_line);

               if (printer_status != PRT_DONE)
                  ReportPrinterError ();

               WriteEOL();
               lines_printed++;
               if ( active_pr.blank_lines_byte > 0)
               {
                    for ( idx2 = 1; idx2 <= active_pr.blank_lines_byte; idx2++)
                        WriteEOL();   /* Changed so that blanks lines etc.
                                         don't go through SendPrinterC();
                                      */
                    lines_printed += active_pr.blank_lines_byte;
                }
            }
        } /* IF */
        else
            ReportPrinterError();
    }
    fclose (file_ptr);
    restore_area (25, 10, 56, 16, print_buff);
    free ((char *)print_buff);
}

void do_print ()
{
    char *do_print_buff;
    int   done, k;

    do_print_buff = (char *)malloc(PORTSIZE(18, 8, 67, 20));
    if ( do_print_buff == NULL)
        return;
    save_area ( 18, 8, 67, 20, do_print_buff);
    SetColourPair ( palette.black, palette.green);
    draw_window ( 18, 8, 66, 19);
    window_title ( 18, 66, 8, "Review Printer Settings", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString ( 21, 10, "Mark Secret Data");
    WriteString ( 21, 11, "Line Terminator");
    WriteString ( 21, 12, "Throw Page");
    WriteString ( 26, 13, "Using");
    WriteString ( 21, 14, "Port");
    WriteString ( 21, 15, "Page Header");
    WriteString ( 21, 16, "Suppression");
    SetColourPair (palette.red, palette.green);
    WriteChar (21, 10, 'M');
    WriteChar (21, 11, 'L');
    WriteChar (21, 12, 'T');
    WriteChar (26, 13, 'U');
    WriteChar  (26, 15, 'H');
    WriteChar ( 21, 14, 'P');
    WriteChar ( 21, 16, 'S');
    draw_button (30, 18, 4, "&Ok", BUTTON_UP); 
    draw_button (45, 18, 8, "&Cancel", BUTTON_UP);
    done = FALSE;
    memcpy (&old_options.eol_terminator, &print_options.eol_terminator, sizeof (struct print_time));

    while  (! done)
    {
        show_print_time_options ();
        k = get_key();
        switch ( k )
        {
            case 'M'   :
            case 'm'   : print_options.mark_secret = !print_options.mark_secret;
                         break;
            case 'S'   :
            case 's'   : print_options.suppress_mode++;
                         if (print_options.suppress_mode > SUPPRESS_NON_ASCII)
                            print_options.suppress_mode = NO_SUPPRESSION;
                         break;
            case K_ESC : 
            case 'C'   :
            case 'c'   : done = TRUE;
                         if (memcmp (&print_options.eol_terminator, &old_options.eol_terminator, sizeof (struct print_time)) != 0)
                         {
                             if (abandon_option_changes())
                                memcpy (&print_options.eol_terminator, &old_options.eol_terminator, sizeof (struct print_time));
                         }
                         simulate_push (45, 18, "&Cancel", 8);
                         push_wait();
                         break;
            case 'H'   :
            case 'h'   : print_options.header_mode++;
                         if (print_options.header_mode > PRINTER_ALL_PAGES)
                            print_options.header_mode = PRINTER_NO_HEADER;
                         break;
            case 'L'    : 
            case 'l'    : print_options.eol_terminator++;
                          if ( print_options.eol_terminator > EOL_CRLF)
                              print_options.eol_terminator = EOL_NONE;
                          break;
            case 'T'    : 
            case 't'    : print_options.throw_page = ! print_options.throw_page;
                          break;
            case 'U'    : 
            case 'u'    : if ( print_options.throw_page)
                          {
                               if ( print_options.using == THROW_USING_FF)
                                   print_options.using = THROW_USING_LF;
                               else
                                   print_options.using = THROW_USING_FF;
                          }
                          break;
            case 'P'    :
            case 'p'    : 
                          print_options.printer_port++;
                          if (print_options.printer_port > PRINTER_LPT4)
                             print_options.printer_port = PRINTER_LPT1;
                          break;
            case 'O'    :
            case 'o'    :
            case K_CR   : done = TRUE;
                          simulate_push (30, 18, "&Ok", 4);
                          push_wait();
            default     : break;
        }
    }
    restore_area (18, 8, 67, 20, do_print_buff);
    free((char *)do_print_buff);
}

void ReportPrinterError ()
{
    char *pr_err_buff;

    if ( printer_status != PRT_DONE )
    {
        pr_err_buff = (char *)malloc(PORTSIZE(28, 10, 53, 14));
        if (pr_err_buff==NULL)
            return;
        save_area (28,10,53,14,pr_err_buff);
        SetColourPair (palette.bright_white, palette.red);
        draw_window (28, 10, 52, 13);
        SetColourPair (palette.yellow, palette.red);
        WriteString  (31, 11, "** PRINTER ERROR **");
        switch ( printer_status & 255 ) /* Only concerned with low byte */
        {
            case PRT_TIMEOUT     : WriteString (37, 12, "TIMEOUT");
                                   break;
            case PRT_BAD_PRINTER : WriteString (35, 12, "NO RESPONSE");
                                   break;
            case PRT_BAD_WRITE   : WriteString (36, 12, "I/O ERROR");
                                   break;
            default              : WriteString (37, 12, "UNKNOWN");
                                   break;
        }
        get_key();
        restore_area (28, 10, 53, 14, pr_err_buff);
        free ((char *)pr_err_buff);
    }
}

unsigned int InitPrinter ( int port )
{
    /* The third parameter - 0 is ignore with this service */

    return (_bios_printer ( _PRINTER_INIT, ( unsigned int )port, 0));
}

/* This does not use SendPrinterC() so that a new line is always sent
   irrespective of tghe limitations on the line width */

unsigned int WriteEOL ()
{
    printer_status = PRT_DONE;
    switch ( print_options.eol_terminator)   
    {
        case EOL_NONE  : /* nothing */
                         break;
        case EOL_CR    : printer_status = _bios_printer (_PRINTER_WRITE, (unsigned int)print_options.printer_port, (unsigned int)active_pr.cr_byte);
                         if ( printer_status != PRT_DONE)
                             ReportPrinterError();
                         break;
        case EOL_LF    : printer_status = _bios_printer ( _PRINTER_WRITE, (unsigned int)print_options.printer_port, (unsigned int)active_pr.lf_byte);
                         if ( printer_status != PRT_DONE)
                                ReportPrinterError();
                         break;
        case EOL_CRLF  : printer_status = _bios_printer ( _PRINTER_WRITE, (unsigned int)print_options.printer_port, (unsigned int)active_pr.cr_byte);
                         if ( printer_status == PRT_DONE)
                         {
                             printer_status = _bios_printer (_PRINTER_WRITE, (unsigned int)print_options.printer_port, (unsigned int)active_pr.lf_byte);
                             if ( printer_status != PRT_DONE)
                                 ReportPrinterError ();
                         }
                         else
                             ReportPrinterError ();
                         break;
      }
     return ( printer_status);
}

unsigned int SendPrinterC ( int port, char ch )
{
    if ( chars_printed < active_pr.line_length_byte)
    {
        printer_status = _bios_printer ( _PRINTER_WRITE, ( unsigned int )port, ( unsigned int )ch );
        if ( printer_status == PRT_DONE)
            chars_printed++;
    }
    else
        printer_status = PRT_DONE; /* No char sent so it must have gone ok !! */
    return ( printer_status);
}

unsigned int SendPrinterS ( int port, char str[])
{
    int i;    
    
    i = 0;
    printer_status = PRT_DONE; /* Set up as OK To Print */
    while ( ( i < strlen (str)) && (printer_status == PRT_DONE))
        printer_status = SendPrinterC(port, str[i++]);
    return ( printer_status );
}

void set_init_print_time_options()
{
    print_options.mark_secret    = TRUE;
    print_options.eol_terminator = EOL_CRLF;
    print_options.throw_page     = TRUE;
    print_options.using          = THROW_USING_FF;
    print_options.printer_port   = PRINTER_LPT1;
    print_options.header_mode    = PRINTER_ALL_PAGES;
    print_options.suppress_mode  = SPACE_REPLACEMENT;
}

void show_print_time_options ()
{
    char str[25];

    SetColourPair (palette.bright_white, palette.green);
    if (print_options.mark_secret)
       WriteString (39,10, "Yes");
    else
       WriteString(39, 10, "No ");
    write_blanks ( 39, 11, 26);
    switch (print_options.eol_terminator)
    {
        case EOL_NONE   : 
                          WriteString ( 39, 11, "None          ");
                          break;
        case EOL_CR     : sprintf ( str, "CR Only (%2d)      ", active_pr.cr_byte);
                          WriteString ( 39, 11, str);
                          break;
        case EOL_LF     : sprintf ( str, "LF Only (%2d)      ", active_pr.lf_byte);
                          WriteString ( 39, 11, str);
                          break;
        case EOL_CRLF   : sprintf ( str, "Both CR And LF (%2d/%2d)", active_pr.cr_byte, active_pr.lf_byte);
                          WriteString ( 39, 11, str);
                          break;
    }
    if ( print_options.throw_page)
    {
        WriteString ( 39, 12, "Yes");

        SetColourPair (palette.black, palette.green);
        WriteString (26, 13, "Using");
        SetColourPair (palette.red, palette.green);
        WriteChar (26, 13, 'U');
        SetColourPair ( palette.bright_white, palette.green);
        write_blanks (39, 13, 20);
        if ( print_options.using == THROW_USING_FF)
        {
            sprintf ( str, "Form Feed (%2d)", active_pr.ff_byte);
            WriteString ( 39, 13, str);
        }
        else
        {
            sprintf ( str, "Line Feed (%2d)", active_pr.lf_byte);
            WriteString ( 39, 13, str);
        }
    }
    else
    {
        WriteString ( 39, 12, "No ");
        write_blanks ( 26, 13, 30);
    }
    switch ( print_options.printer_port )
    {
        case PRINTER_LPT1   : WriteString ( 39, 14, "LPT1");
                              break;
        case PRINTER_LPT2   : WriteString ( 39, 14, "LPT2");
                              break;
        case PRINTER_LPT3   : WriteString ( 39, 14, "LPT3");
                              break;
        case PRINTER_LPT4   : WriteString ( 39, 14,"LPT4");
                              break;
    }
    switch ( print_options.header_mode )
    {
        case PRINTER_ALL_PAGES  : WriteString ( 39, 15, "All Pages ");
                                  break;
        case PRINTER_FIRST_PAGE : WriteString ( 39, 15, "First Page");
                                  break;
        case PRINTER_NO_HEADER  : WriteString ( 39, 15, "None      ");
                                  break;
    }
    switch (print_options.suppress_mode)
    {
       case NO_SUPPRESSION      : WriteString (39, 16, "None                  ");
                                  break;
       case SPACE_REPLACEMENT   : WriteString (39, 16, "Replace With Spaces   ");
                                  break;
       case SUPPRESS_NON_ASCII  : WriteString (39, 16, "Suppress Non-Printable");
                                  break;
    }
}

void show_port (int port_num, int state)
{
   char str[5];

   if (state)
      SetColourPair (palette.bright_white, palette.black);
   else
      SetColourPair (palette.black, palette.green);
   sprintf (str, "LPT%d", port_num);
   WriteString (46, 13+port_num, str);
   if (! state)
   {   
      sprintf (str, "%d", port_num);
      SetColourPair (palette.red, palette.green);
      WriteString (49,13+port_num, str);
   }
}

void throw_page ()
{
    int i;

    i = lines_printed;

    if ( print_options.throw_page )   
    {
        if ( print_options.using == THROW_USING_LF )
        {
            while ( ( i <= active_pr.page_length_byte) && (printer_status == PRT_DONE))
            {
                printer_status = SendPrinterC ( print_options.printer_port, active_pr.lf_byte);
                i++;
                if ( printer_status != PRT_DONE)
                    ReportPrinterError();
            }
        }
        else
        {
            printer_status = SendPrinterC ( print_options.printer_port, active_pr.ff_byte);
            if ( printer_status != PRT_DONE)
                ReportPrinterError();
        }
    }
    page++;
}

