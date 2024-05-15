#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <bios.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>

#include "keys.h"
#include "palette.h"
#include "windows.h"
#include "iq.h"
#include "printer.h"

#define PAGE_HEADER1     "IQuest v1.1 (c) 1996                                          Page %3u"  
#define PAGE_HEADER2     "----------------------------------------------------------------------"

#define PRT_TIMEOUT      1
#define PRT_DONE         144
#define PRT_BAD_PRINTER  48
#define PRT_BAD_WRITE    49


unsigned int printer_status;

FILE *pr_ptr;
int page;
int traverse_dir;

struct print_time
{
    int           field_order;
    int           eol_terminator;
    int           throw_page;
    int           using;
    int           first,
                  last;
    unsigned char printer_port;
    int           header_mode;
} print_options;

char mnemonic[32][4] = {"NUL","SOH","STX","ETX","EOT","ENQ","ACK","BEL",
                        "BS","HT","LF","VT","FF","CR","SO","SI","DLE",
                        "DC1","DC2","DC3","DC4","NAK","SYN","ETB",
                        "CAN","EM","SUB","ESC","FS","GS","RS","US"
                       };

extern struct printer_str basic_pr;

extern int found_search_matches;
extern char exp_name[];
extern struct palette_str palette;

extern void window_title ( int x1, int x2, int y, char msg[], short fg, short bk);
extern unsigned int draw_button ( int x, int y, int width, char msg[], int state);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern void simulate_push (int x, int y, char msg[], int width);
extern void push_wait ( void );
extern void GotoXY (short x, short y);
extern int ascii_table ( void );
extern unsigned char getstr ( unsigned int x, unsigned int y, char str[], char sp[], int length);
extern int get_yesno(int, int, short, short);
extern void get_name_stub (char *);
extern void flush_keyboard( void );
extern void write_blanks (int, int, int);
extern unsigned int get_key ( void );
extern int file_exists ( char name[]);
extern void centre_push_button ( int x1, int x2, int y, char msg[]);

struct find_t c_file;
unsigned int lines_printed,
             chars_printed;

struct pr_file_str
{
    char filename[13];
    char description[MAX_PR_DESC_SIZE+1];
} pr_list[MAX_PR_FILES];

int pr_idx;

struct field_place
{
    int x,
        y,
        width;
}; 

struct field_place field_pos[NUMBER_OF_FIELDS] = {{35, 9, MAX_PR_DESC_SIZE}, {35, 10, 20},
                                                  {35, 12, 3}, {59, 12, 3},
                                                  {35, 13, 3},
                                                  {35, 15, 3}, {59, 15, 3},
                                                  {35, 16, 3}
                                                 };

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

struct printer_str active_pr;
char pr_filename[13];

/* Routines To Output Phone List To Printer */


void         save_pr_file (char []);
void         build_pr_list ( void );
unsigned int WriteEOL ( void );
void         build_pr_list ( void );
void         set_empty_pr_file ( void );
void         read_pr_desc ( void );
void         ShowAllMnemonics ( void );
int          get_mnemonic ( void );   
void         read_pr_init_string ( void );
void         show_more_pr_help ( void );
void         show_printer_settings ( void );   
void         do_pr_config_help ( void );
int          ask_save_config ( void );   
void         show_mnemonic_help ( void );   
void         get_printer_init_string ( void );
void         do_printer_config ( void );
void         get_pr_window ( void );
void         put_pr_window ( void );   
void         do_load_pr_settings ( void );
void         do_save_pr_settings ( void );   
void         invalid_filename ( void );
void         throw_page ( void );
void         show_print_range ( void );   
void         select_print_range ( void );   
void         get_print_range ( void );   
void         get_printer_port ( void );   


void get_pr_desc ( char *name)
{
    FILE *tmp;
    char  str[MAX_PR_DESC_SIZE+1];

    tmp = fopen (name, "rb");
    fread ( str, MAX_PR_DESC_SIZE, 1, tmp);
    fclose ( tmp );
    strcpy ( pr_list[pr_idx].description, str);
}

void build_pr_list()
{
   char *no_pr_buff;

   pr_idx = 0;
   if ( _dos_findfirst ( "*.PR", FILE_ATTR_LIST, &c_file) == 0) 
   {
      strcpy ( pr_list[pr_idx].filename, c_file.name);   
      get_pr_desc (c_file.name);
      pr_idx++;
      while ( _dos_findnext (&c_file) == 0)
      {
          if ( pr_idx < MAX_PR_FILES)
          {
              strcpy ( pr_list[pr_idx].filename, c_file.name);
              get_pr_desc (c_file.name);
              pr_idx++;
          }
      }
      if ( pr_idx == MAX_PR_FILES)
      {
      }
   }
   else
   {
       no_pr_buff = (char *)malloc(PORTSIZE(25, 10, 56, 15));
       if ( no_pr_buff == NULL)
           return;
       save_area ( 25, 10, 56, 15, no_pr_buff);
       SetColourPair (palette.bright_white, palette.red);
       draw_window (25, 10, 55, 14);
       SetColourPair (palette.yellow, palette.red);
       WriteString ( 30, 11, "No Printer Files Found");
       centre_push_button (25, 54, 13, "&Continue");
       restore_area ( 25, 10, 56, 15, no_pr_buff);
       free((char *)no_pr_buff);
   }
}

void set_empty_pr_file ()
{
    memcpy ( &active_pr.pr_description[0], &basic_pr.pr_description[0], sizeof ( struct printer_str ));
    strcpy ( pr_filename, "GENERIC.PR");
}

void read_pr_desc ()
{
    int           idx;
    unsigned char ch;

    idx = 0;
    ch  = ( unsigned char )fgetc ( pr_ptr );
    while (( ch != ( unsigned char )0 ) && ( idx < MAX_PR_DESC_SIZE )) 
    {
        active_pr.pr_description[ idx++ ] = ch;
        ch = ( unsigned char )fgetc ( pr_ptr );
    }
    active_pr.pr_description[ idx ] = ( unsigned char )0;
}

void read_pr_init_string ()
{
   unsigned char ch;
   int           idx;

   idx = 0;
   ch  = ( unsigned char )fgetc ( pr_ptr );
   while ( ( ch != ( unsigned char )0) && ( idx < MAX_PR_INIT_SIZE ))
   {
       active_pr.pr_init_string[ idx++ ] = ch;
       ch = ( unsigned char )fgetc ( pr_ptr );
   }
   active_pr.pr_init_string[ idx ] = ( unsigned char )0;
}

void load_pr_file ( char *filename)
{
    pr_ptr = fopen ( filename, "rb" );

    read_pr_desc ();
    active_pr.lf_byte = ( unsigned char )fgetc ( pr_ptr );
    active_pr.ff_byte = ( unsigned char )fgetc ( pr_ptr );
    active_pr.cr_byte = ( unsigned char )fgetc ( pr_ptr );
    read_pr_init_string ();
    active_pr.line_length_byte = ( unsigned char )fgetc ( pr_ptr );
    active_pr.page_length_byte = ( unsigned char )fgetc ( pr_ptr );
    active_pr.blank_lines_byte = ( unsigned char )fgetc ( pr_ptr );
    fclose ( pr_ptr );
}

void save_pr_file ( char *filename )
{
    pr_ptr = fopen ( filename, "wb" );
    fprintf ( pr_ptr, "%s%c%c%c%c%s%c%c%c%c", active_pr.pr_description, ( unsigned char )0,
                                              active_pr.lf_byte, active_pr.ff_byte,
                                              active_pr.cr_byte, active_pr.pr_init_string, ( unsigned char )0,
                                              active_pr.line_length_byte, active_pr.page_length_byte, active_pr.blank_lines_byte
            );
    fclose ( pr_ptr );
}

void ShowMnemonic(int i, int state)
{
   char str[20];

   if (state)
      SetColourPair (palette.bright_white, palette.green);
   else
      SetColourPair (palette.black, palette.green);
   sprintf ( str, "%02d <%s>", i, mnemonic[i]);    
   switch ( i / 8)
   {
       case 0 : WriteString ( 28, i+6, str);
                break;  
       case 1 : WriteString ( 39, (i-8)+6, str);
                break;
       case 2 : WriteString ( 50, (i-16)+6, str);
                break;
       case 3 : WriteString ( 61, (i-24)+6, str);
                break;
   }
}

void ShowAllMnemonics()
{
    int i;

    for (i = 0; i < 32; i++)
        ShowMnemonic(i, FALSE);
}

int get_mnemonic ()
{
    char *nm_buff;
    int  choice, chosen_one;
    unsigned int key;

    nm_buff = (char *)malloc(PORTSIZE(25, 3, 73, 16));
    if (nm_buff == NULL)
       return(-1);
    save_area (25, 3, 73, 16, nm_buff);
    hide_cursor();
    SetColourPair (palette.black, palette.green);
    draw_window (25, 3, 72, 15);
    ShowHeader (25, 72, 4, "ASCII Mnemonic Codes");
    ShowAllMnemonics();
    chosen_one = FALSE;
    choice=0;
    while (! chosen_one)
    {
        ShowMnemonic (choice, TRUE);    
        key = get_key();
        ShowMnemonic (choice, FALSE);
        switch ( key )
        {
            case K_ESC       : choice = -1;
                               chosen_one = TRUE;
                               break;
            case K_CR        : chosen_one = TRUE;
                               break;
            case K_CSR_LT    : if (choice >= 8)
                                   choice-=8;
                               break;
            case K_CSR_RT    : if ( choice < 24)
                                   choice+=8;
                               break;
            case K_CSR_UP    : if (choice > 0)
                                  choice--;
                               break;
            case K_CSR_DN    : if (choice < 31)
                                   choice++;
                               break;
            default          : break;
        }
    }
    restore_area (25, 3, 73, 16, nm_buff);
    free ((char *)nm_buff);
    show_cursor();
    return ( choice);
}

void show_printer_settings ()
{
    char ascii[4];
    char tmp[80];
    SetColourPair ( palette.bright_white, palette.black);
    WriteString ( 35, 9, active_pr.pr_description );
    
    strcpy ( tmp, active_pr.pr_init_string);
    if ( strlen ( tmp) > 20)
        tmp[20] = (char)0;
    WriteString ( 35, 10, tmp );
    itoa ( active_pr.lf_byte, ascii, 10); WriteString ( 35, 12, ascii );
    itoa ( active_pr.ff_byte, ascii, 10); WriteString ( 59, 12, ascii );
    itoa ( active_pr.cr_byte, ascii, 10); WriteString ( 35, 13, ascii );
    itoa ( active_pr.line_length_byte, ascii, 10); WriteString ( 35, 15, ascii );
    itoa ( active_pr.page_length_byte, ascii, 10); WriteString ( 59, 15, ascii );
    itoa ( active_pr.blank_lines_byte, ascii, 10); WriteString ( 35, 16, ascii );
}


void print_field ( int field )
{
    char byte[4];
    char tmp[MAX_PR_INIT_SIZE+1];

    SetColourPair (palette.bright_white, palette.black);
    write_blanks ( field_pos[field].x, field_pos[field].y, field_pos[field].width);
    
    switch ( field )
    {
       case 0 : WriteString ( field_pos[0].x, field_pos[0].y, active_pr.pr_description);
                break;
       case 1 : strcpy ( tmp, active_pr.pr_init_string);
                if ( strlen ( tmp) > 20)
                    tmp[20] = (char)0;
                WriteString ( field_pos[1].x, field_pos[1].y, tmp);
                break;
       case 2 : itoa ( active_pr.lf_byte, byte, 10);
                WriteString ( field_pos[2].x, field_pos[2].y, byte);
                break;
       case 3 : itoa ( active_pr.ff_byte, byte, 10);
                WriteString ( field_pos[3].x, field_pos[3].y, byte);
                break;
       case 4 : itoa ( active_pr.cr_byte, byte, 10);
                WriteString ( field_pos[4].x, field_pos[4].y, byte);
                break;
       case 5 : itoa ( active_pr.line_length_byte, byte, 10);
                WriteString ( field_pos[5].x, field_pos[5].y, byte);
                break;
       case 6 : itoa ( active_pr.page_length_byte, byte, 10);
                WriteString ( field_pos[6].x, field_pos[6].y, byte);
                break;
       case 7 : itoa ( active_pr.blank_lines_byte, byte, 10);
                WriteString ( field_pos[7].x, field_pos[7].y, byte);
                break;
    }

}

void show_more_pr_help()
{
    char *more_pr_help_buff;

    more_pr_help_buff = (char *)malloc(PORTSIZE(19, 4, 75,22));
    if ( more_pr_help_buff == NULL)
       return;
    save_area (19, 4, 75, 22, more_pr_help_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (19, 4, 74, 21);
    window_title ( 19, 74, 4, "Further Help", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString (22,  6, "Description   - Name describing chosen printer.");
    WriteString (22,  7, "Init String   - Setup characters sent to printer");
    WriteString (22,  8, "                before printing begins.");
    WriteString (22,  9, "Linefeed      - Character sent to printer at the");
    WriteString (22, 10, "                end of each LINE. (Usually 10).");
    WriteString (22, 11, "Formfeed      - Character sent to printer at the");
    WriteString (22, 12, "                end of each PAGE. (Usually 12).");
    WriteString (22, 13, "Carr. Return  - Character sent to printer at the");
    WriteString (22, 14, "                end of each LINE. (Usually 13).");
    WriteString (22, 15, "Line Length   - Number of characters per line.");
    WriteString (22, 16, "Page Length   - Number of characters per page.");
    WriteString (22, 17, "Blank Lines   - Number Of Blank Lines Between");
    WriteString (22, 18, "                Records. (Usually 0).");
    centre_push_button ( 19, 74, 20, "&Ok");
    SetColourPair (palette.black, palette.green);
    restore_area (19, 4, 75, 22, more_pr_help_buff);
    free ((char *)more_pr_help_buff);
}

void do_pr_config_help()
{
    char *pr_config_help_buff;
    unsigned int key;
    int finish;

    pr_config_help_buff = (char *)malloc(PORTSIZE (14, 8, 67, 20));
    if ( pr_config_help_buff == NULL)
        return;
    save_area (14, 8, 67, 20, pr_config_help_buff);
    SetColourPair ( palette.black, palette.green);
    draw_window (14, 8, 66, 19);
    window_title (14, 66, 8, "Configuration Help", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString ( 17, 10, "F1         -   Display This Help Page.");
    WriteString ( 17, 11, "ESC        -   Leave Printer Configuration.");
    WriteString ( 17, 12, "CSR-UP/DN  -   Move to Previous/Next Field.");
    WriteString ( 17, 13, "CTRL-A     -   Select Character From ASCII");
    WriteString ( 17, 14, "               Table (Only For Numeric Fields).");
    WriteString ( 17, 15, "TAB        -   Move To Next Field.");
    draw_button (38, 17, 4, "&Ok", BUTTON_UP);
    SetColourPair (palette.bright_white, palette.green);
    WriteString ( 25, 19, " Press F1 Again For Further Help ");
    SetColourPair (palette.red, palette.green); 
    WriteString ( 32, 19, "F1");
    finish = FALSE;
    while (! finish)
    {
       key = get_key();
       switch ( key)
       {
          case K_CR     :
          case 'O'      : 
          case 'o'      : finish = TRUE;
                          simulate_push (38, 17, "&Ok", 4);
                          push_wait();
                          break;
          case K_F1     : show_more_pr_help();
                          break;
          default       : break;

       }
    }
    restore_area (14, 8, 67, 20, pr_config_help_buff);
    free ((char *)pr_config_help_buff);
}

int string_is_numeric ( char *str )
{
   int retval;
   int idx;

   retval = TRUE;
   idx = 0;
   while ( retval && ( idx < strlen ( str)))
   {
       if ( isdigit (str[idx]))
           idx++;
       else
          retval = FALSE;
   }
   return ( retval);
}

void bad_field ( int field_num, int type)
{
    char *bad_field_buff;

    bad_field_buff = (char *)malloc (PORTSIZE(25, 7, 56, 10));
    if ( bad_field_buff == NULL )
        return;
    save_area (25, 7, 56, 10, bad_field_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25, 7, 55, 9);
    SetColourPair (palette.yellow, palette.red );
    switch ( field_num )
    {
        case 0  : WriteString ( 28, 8, "A Description Is Required");
                  break;
        case 1  : break;
        default : if ( ! type)
                      WriteString ( 34, 8, "Invalid Entry");
                  else
                      WriteString ( 30, 8, "Value Is Out Of Range");
                  break;
    }
    get_key();
    print_field ( field_num );
    restore_area ( 25, 7, 56, 10, bad_field_buff);
    free ((char *)bad_field_buff);
}

void set_new_field_value ( int field_num, char str[])
{
    switch ( field_num)
    {
        case 0  : strcpy ( active_pr.pr_description, str);
                  break;
        case 1  : strcpy ( active_pr.pr_init_string, str);
                  break;
        case 2  : active_pr.lf_byte = (unsigned char)atoi (str);
                  break;
        case 3  : active_pr.ff_byte = (unsigned char)atoi(str);
                  break;
        case 4  : active_pr.cr_byte = (unsigned char)atoi(str);
                  break;
        case 5  : active_pr.line_length_byte = (unsigned char)atoi(str);
                  break;
        case 6  : active_pr.page_length_byte = (unsigned char)atoi(str);
                  break;
        case 7  : active_pr.blank_lines_byte = (unsigned char)atoi(str);
    }
}


int validate_entry ( int field, char *entry)
{
       int int_val;
       int retval;

       retval = TRUE;
       switch ( field )
       {
           case 0  :
                     /* We must have a description of some sort for later on */

                     if ( ( strlen ( entry ) == 0) && (strlen ( active_pr.pr_description ) == 0))
                     {
                         bad_field (field, TRUE );
                         retval = FALSE;
                     }
                     else
                     {
                         set_new_field_value ( field, entry);
                         print_field (field );
                     }
                     break;

           case 1  :

                     /* Anything goes in the printer init field */

                     set_new_field_value ( field, entry );
                     print_field (field);
                     break;

           default :
                     /* all other fields must  be valid ASCII chars */

                     if ( string_is_numeric ( entry ))
                     {
                         int_val = atoi ( entry);
                         if ( ( int_val >= 0)  && ( int_val <= 255))
                         {
                             /* It's OK */

                             set_new_field_value ( field, entry);
                             print_field (field);
                         }
                         else
                         {
                             bad_field ( field, TRUE );
                             retval = FALSE;
                         }
                     }
                     else
                     {
                         bad_field ( field,  FALSE );
                         retval = FALSE;
                     }
                     break;
       }
       return ( retval);
}

int ask_save_config ()
{
    char         *save_buff;
    int          retval;

    save_buff = (char *)malloc(PORTSIZE(30, 8, 51, 12));
    if ( save_buff == NULL )
        return ( FALSE); /* if we'er outta memory then dont save changes */
    save_area ( 30, 8, 51, 12, save_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window ( 30, 8, 50, 11);
    SetColourPair (palette.yellow, palette.red);
    WriteString (34, 9, "Save Changes ?");
    retval = get_yesno (34, 10, palette.yellow, palette.red);
    restore_area (30, 8, 51, 12, save_buff);
    free ((char *)save_buff);
    return (retval);
}

void show_mnemonic_help()
{
    char *mn_help_buff;

    mn_help_buff = (char *)malloc(PORTSIZE (13, 6, 68, 17));
    if (mn_help_buff == NULL)
        return;
    save_area (13, 6, 68, 17, mn_help_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (13, 6, 67, 16);
    window_title ( 13, 67, 6, "Printer Initialisation Help", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString ( 17,  8, "F1         - Display This Help Page.");
    WriteString ( 17,  9, "ESC        - Abort Entry.");
    WriteString ( 17, 10, "CSR-UP/DN  - Move To Previous/Next Field.");
    WriteString ( 17, 11, "ALT-M      - Select Code From Mnemonic Table");
    WriteString ( 17, 12, "DEL/BS     - Delete Last Character.");
    WriteString ( 17, 13, "ENTER      - Confirm Initialisation String");
    centre_push_button ( 13, 63, 15, "&Ok");
    restore_area (13, 6, 68, 17, mn_help_buff);
    free ((char *)mn_help_buff);
}

void get_printer_init_string ()
{
    char *init_buff;
    char new_init[255];
    int state, ch;
    char str[2];
    int done,
        init_ofs;

    init_buff = (char *)malloc(PORTSIZE(20, 8, 61, 14));
    if ( init_buff == NULL)
        return;
    save_area (20, 8, 61, 14, init_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (20, 8, 60, 13);
    SetColourPair (palette.red, palette.blue);
    WriteString ( 31, 13, " Press F1 For Help ");
    SetColourPair (palette.yellow, palette.blue);
    WriteString (26, 9, "Enter Printer Initialisation");
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (23, 11, MAX_PR_INIT_SIZE);

    memset (new_init, 0, MAX_PR_INIT_SIZE + 1);
    strcpy ( new_init, active_pr.pr_init_string);
    init_ofs = strlen ( new_init );
    done = FALSE;
    show_cursor();
    while (! done )
    {
         GotoXY (23+init_ofs, 11); 
         SetColourPair (palette.bright_white, palette.black);
         write_blanks (23, 11, MAX_PR_INIT_SIZE);
         WriteString (23, 11, new_init);
         state = get_key();
         switch ( state )
         {
             case K_F1     : hide_cursor();
                             show_mnemonic_help();
                             show_cursor();
                             break;
             case K_DEL    : 
             case K_BS     : if ( init_ofs > 0)
                             {
                                  new_init[init_ofs-1] = (char)0;
                                  init_ofs--;
                             }
                             break;
             case K_ESC    : done = TRUE;
                             traverse_dir = 1;
                             break;
             case K_CSR_UP : traverse_dir = -1;
                             done = TRUE;
                             break;
             case K_CSR_DN : 
             case K_CR     : traverse_dir = 1;
                             done = TRUE;
                             break;
             case K_ALT_M  : 
                             ch = get_mnemonic(); 
                             if ( ch != -1) /* Not ESCaped Out */
                             {
                                 str[0] = (char)ch;
                                 str[1] = (char)0;
                                 strcat ( new_init, str);
                                 init_ofs = strlen ( new_init );
                             }
                             break;
             default       : if ( ( state < 128) && ( strlen ( new_init) < MAX_PR_INIT_SIZE))
                             {
                                 str[0] = (char)state;
                                 str[1] = (char)0;
                                 strcat ( new_init, str);
                                 init_ofs = strlen ( new_init);
                             }
                             break;
        }
    }
    
    strcpy ( active_pr.pr_init_string, new_init);

    restore_area (20, 8, 61, 14, init_buff);
    free ((char *)init_buff);
    hide_cursor();
}

void do_printer_config()
{
    char          *printer_config_buff;
    char          entry[80]; 
    int           field,
                  done;
    unsigned char k;
    char          hotkeys[10];
    struct        printer_str tmp_printer;


    printer_config_buff = (char *)malloc(PORTSIZE (15, 7, 66, 19));
    if ( printer_config_buff == NULL)
        return;

    /* Backup printer config in case we decide to abort or quit */

    memcpy ( &tmp_printer.pr_description[0], &active_pr.pr_description[0], sizeof ( struct printer_str));

    save_area ( 15, 7, 66, 19, printer_config_buff );
    SetColourPair ( palette.black, palette.green );
    draw_window ( 15, 7, 65, 18);
    window_title ( 15, 65, 7, "Printer Configuration", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString ( 18, 9, "Description      ");
    WriteString ( 18, 10, "Init String      ");
    WriteString ( 18, 12, "Line Feed        ");
    WriteString ( 43, 12, "Form Feed        ");
    WriteString ( 18, 13, "Carriage Return  ");
    WriteString ( 18, 15, "Line Length      ");
    WriteString ( 43, 15, "Page Length      ");
    WriteString ( 18, 16, "Blank Lines      ");
    SetColourPair (palette.yellow, palette.green );
    WriteChar ( 34,  9, '['); WriteChar ( 34 + 1 + strlen ( BLANK_PR_DESC ), 9, ']');
    WriteChar ( 34, 10, '['); WriteChar ( 34 + 1 + strlen ( BLANK_PR_INIT ), 10, ']'); 
    WriteString ( 34, 12, "[   ]"); WriteString ( 58, 12, "[   ]");
    WriteString ( 34, 13, "[   ]");
    WriteString ( 34, 15, "[   ]"); WriteString ( 58, 15, "[   ]");
    WriteString ( 34, 16, "[   ]");
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (35, 9, MAX_PR_DESC_SIZE);
    write_blanks (35, 10, 20);
    WriteString ( 35, 12, "   "); WriteString ( 59, 12, "   ");
    WriteString ( 35, 13, "   ");
    WriteString ( 35, 15, "   "); WriteString ( 59, 15, "   ");
    WriteString ( 35, 16, "   ");
    show_printer_settings ();
    
    done = FALSE;
    field = 0; /* Start at printer description */
    hotkeys[0] = (char)K_ESC; hotkeys[1] = (char)K_F1; hotkeys[2] = (char)K_F1;
    hotkeys[3] = (char)K_CSR_UP; hotkeys[4] = (char)K_CSR_DN;
    hotkeys[5] = (char)K_CTRL_A; hotkeys[6]=(char)0;

    entry[0] = (char)0;
    while (! done )
    {
        if ( field < 2) /* Text Field so don't allow CTRL-A Hotkey */
            hotkeys[5] =(char)0;
        else
            hotkeys[5]=(char)K_CTRL_A;
            
        SetColourPair (palette.bright_white, palette.black);
        write_blanks ( field_pos[field].x, field_pos[field].y, field_pos[field].width);
        if ( field == 1) /* Printer Init String */
        {
            get_printer_init_string();
            strcpy ( entry, active_pr.pr_init_string); /* Trim to fit screen */
            entry[20] = (char)0;
            WriteString ( field_pos[field].x, field_pos[field].y, entry);
            field += traverse_dir; 
            write_blanks (field_pos[field].x, field_pos[field].y, field_pos[field].width);
            entry[0] = (char)0;
        }
        else
        {
            k = getstr ( field_pos[field].x, field_pos[field].y, entry, hotkeys, field_pos[field].width);
            if ( strchr (hotkeys, k) != NULL)
            {
                switch ( k )
                {
                    case K_ESC      : done = TRUE;
                                      break;
                    case K_F1       : do_pr_config_help();
                                      break;
                    case K_CTRL_A   : /* since hotkey list is predefined based
                                         on field number, no test is needed here */
               
                                      itoa (ascii_table(), entry, 10);
                                      /* No need to validate entry since value will
                                         always be in range and numeric */
                                      set_new_field_value ( field, entry);
                                      print_field (field);
                                      field++;
                                      if ( field == NUMBER_OF_FIELDS)
                                         done = TRUE;
                                      break;
                    case K_CSR_UP   : print_field(field);
                                      traverse_dir = -1;
                                      switch ( field )
                                      {
                                           case 1  : field = 0;
                                                     break;
                                           case 2  : field = 1;
                                                     break;
                                           case 3  : field = 1; 
                                                     break;
                                           case 4  : field = 2;
                                                     break;
                                           case 5  : field = 4;
                                                     break;
                                           case 6  : field = 3;
                                                     break;
                                           case 7  : field = 5;
                                                     break;
                                           default : break;
                                      }
                                      break;
                    case K_CSR_DN   : print_field (field);
                                      traverse_dir = +1;
                                      switch ( field )
                                      {
                                           case 0  : field = 1;
                                                     break;
                                           case 1  : field = 2;
                                                     break;
                                           case 2  : field = 4;
                                                     break;
                                           case 3  : field = 6;
                                                     break;
                                           case 4  : field = 5;
                                                     break;
                                           case 5  :
                                           case 6  : field = 7;
                                                     break;
                                           default : break;
                                      }
                                      break;              
                      default       : traverse_dir = +1;
                                      if ( strlen ( entry) > 0)
                                      {
                                          if (validate_entry ( field, entry ))              
                                              field++;
                                      }
                                      else
                                      {
                                          /* Ok, so we're just CRing thru the fields */
                
                                          print_field (field);
                                          field++;
                                          
                                      }
                                      if ( field == NUMBER_OF_FIELDS)
                                          done = TRUE;
                                      break;
                }
            }
            else
            {
                if (validate_entry ( field, entry ))
                    field++;
            }
        }
    }
    show_printer_settings();
    if ( ! ask_save_config ())
        memcpy ( &active_pr.pr_description[0], &tmp_printer.pr_description[0], sizeof ( struct printer_str));
    flush_keyboard();
    restore_area ( 15, 7, 66, 19, printer_config_buff );
    free (( char *)printer_config_buff);
}

void show_pr_entry ( int base, int idx, int state)
{
    if (state)
        SetColourPair (palette.yellow, palette.blue);
    else
       SetColourPair (palette.bright_white, palette.blue);
    WriteString (38, 9+idx, pr_list[base+idx].description); 
    SetColourPair (palette.light_red, palette.blue);
    write_blanks (43, 17, 15);
    WriteString (44, 17, pr_list[base+idx].filename);
}

void show_pr_range ( int start )
{
   int idx;

   for (idx = 0; idx < 6; idx++)
      write_blanks ( 38, 9+idx, MAX_PR_DESC_SIZE);   

   idx = 0;
   SetColourPair (palette.bright_white, palette.blue);
   while ( ( idx < 6)  && (start+idx < pr_idx))
   {
         WriteString ( 38, 9+idx, pr_list[start+idx].description);
         idx++;
   }
}

char *details_buff;

void get_pr_window()
{
   details_buff = (char *)malloc(PORTSIZE(2, 6, 33, 19));
   if ( details_buff != NULL)
   {
       save_area (2, 6, 33, 19, details_buff);
       SetColourPair (palette.black, palette.green);
       draw_window ( 2, 6, 32, 18);
       SetColourPair (palette.black, palette.green);
       WriteString ( 5, 10, "Linefeed Character Is ");
       WriteString ( 5, 11, "Formfeed Character Is ");
       WriteString ( 5, 12, "Return Character Is   ");
       WriteString ( 5, 13, "Page Length Is    Lines");
       WriteString ( 5, 14, "Page Width Is     Chars");
       WriteString ( 5, 15, "Blank Lines / Record Is ");
       WriteString ( 5, 16, "Init String Is ");
   }
}

void put_pr_window ()
{
   if ( details_buff != NULL)
   {
       restore_area ( 2, 6, 33, 19, details_buff);
       free ((char *)details_buff);
   }
}


void show_pr_settings ( int offset )
{
    struct printer_str tmp_pr;
    char heading[30];
    char str[25];

    /* Save active printer settings */

    memcpy ( &tmp_pr.pr_description[0], &active_pr.pr_description[0], sizeof ( struct printer_str ));
    sprintf ( heading, " %s ", pr_list[offset].filename);
    SetColourPair ( palette.red, palette.green);
    write_blanks (11, 6, 13);
    WriteString ( 2+((31 - strlen ( heading ))/2), 6, heading);
    load_pr_file ( pr_list[offset].filename);
    SetColourPair (palette.bright_white, palette.green);
    write_blanks ( 5, 8, 25); write_blanks ( 5, 17, 25);

    if ( strlen ( active_pr.pr_description) > 25)
        strncpy ( str, active_pr.pr_description, 25);
    else
        strcpy ( str, active_pr.pr_description);
    WriteString ( 5, 8, str);
    sprintf ( str, "%3d", active_pr.lf_byte);
    WriteString ( 26, 10, str);
    sprintf ( str, "%3d", active_pr.ff_byte);
    WriteString ( 26, 11, str);
    sprintf ( str, "%3d", active_pr.cr_byte);
    WriteString ( 24, 12, str);
    sprintf ( str, "%2d", active_pr.page_length_byte);
    WriteString ( 20, 13, str);
    sprintf ( str, "%3d", active_pr.line_length_byte);
    WriteString ( 19, 14, str);
    sprintf ( str, "%2d", active_pr.blank_lines_byte);
    WriteString (29, 15, str);
    if ( active_pr.pr_init_string[0] == (char)0)
        strcpy ( str, "   ** Nothing **");
    else
    {
        if ( strlen ( active_pr.pr_init_string) > 25)
           strncpy ( str, active_pr.pr_init_string, 25);
        else
           strcpy ( str, active_pr.pr_init_string);
    }
    WriteString ( 7, 17, str);
    
    memcpy ( &active_pr.pr_description[0], &tmp_pr.pr_description[0], sizeof ( struct printer_str));

}

void do_load_pr_settings ()
{
    char *load_buff;
    int   current, offset, done;
    unsigned int k;

    build_pr_list();
    done = ( pr_idx == 0); /* No .PR Files Found */
    if ( ! done )
    {
        load_buff = (char *)malloc(PORTSIZE(35, 7, 65, 18));
        if ( load_buff == NULL)
            return;
        save_area (35, 7, 65, 18, load_buff);
        SetColourPair (palette.bright_white, palette.blue);
        draw_window (35, 7, 64, 17);
        write_blanks (42, 17, 15);
        SetColourPair (palette.red, palette.blue);
        window_title ( 35, 64, 7, "Load Printer File", palette.yellow, palette.blue);
            
        current = offset = 0;
        show_pr_range (0);
        get_pr_window();
        while (! done)
        {
            show_pr_entry (offset, current, TRUE);

            show_pr_settings(current+offset);
            k = get_key();
            show_pr_entry (offset, current, FALSE);
            switch ( k )
            {
                case K_ESC    : done = TRUE;
                                break;
                case K_CR     : done = TRUE;
                                break;
                case K_CSR_UP : 
                                if ( current > 0)
                                    current--;
                                else
                                {
                                   if ( offset > 0)
                                   {
                                      offset--;
                                      current = 0;
                                      show_pr_range(offset);
                                   }
                                }
                                break;
                case K_CSR_DN : 
                                if ( current + 1 < pr_idx )
                                    current++;
                                if ( current == 6 )
                                {
                                    current = 5;
                                    offset++;
                                    if (offset + current >= pr_idx)
                                        offset--;                                    
                                    show_pr_range (offset);
                                }    
                                break;
                default       : break;
            }
        }
        put_pr_window();
        if ( k == K_CR)
            load_pr_file (pr_list[offset+current].filename);
        restore_area (35, 7, 65, 18, load_buff);
        free ((char *)load_buff);
    }
}

void invalid_filename ()
{
   char *bad_name_buff;

   bad_name_buff = (char *)malloc(PORTSIZE (30, 11, 51, 14));
   if ( bad_name_buff == NULL)
       return;
   save_area (30, 11, 51, 14, bad_name_buff);
   SetColourPair (palette.bright_white, palette.red);
   draw_window (30,11,51,13);
   SetColourPair (palette.yellow, palette.red);
   WriteString ( 33, 12, "Invalid Filename");
   get_key();
   restore_area (30, 11, 51, 14, bad_name_buff);
   free ((char *)bad_name_buff);
}

int overwrite_pr_file (char *name)
{
    char *exists_buff;
    int  retval;
    char str[80];

    exists_buff = (char *)malloc(PORTSIZE(25, 7, 56, 11));
    if (exists_buff == NULL)
        return (FALSE);
    save_area (25, 7, 56, 11, exists_buff);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (25, 7, 55, 10);
    SetColourPair (palette.yellow, palette.red);
    sprintf (str, "Overwrite %s ?", strupr(name));
    WriteString ( 25 + (30-strlen (str))/2, 8, str);
    retval = get_yesno (33,9,palette.yellow, palette.red);
    restore_area (25, 7, 56, 11, exists_buff);
    free ((char *)exists_buff);
    return (retval);
}

void do_save_pr_settings ()
{
    char *save_buff;
    char filename[9];
    char hotkeys[3];
    int state;
    FILE *fp;

    save_buff = (char *)malloc (PORTSIZE(20, 9, 61, 15));
    if ( save_buff == NULL)
        return;
    hotkeys[0] = (char)K_ESC;
    hotkeys[1] = (char)0;
    save_area (20, 9, 61, 15, save_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (20,9,60,14);
    SetColourPair (palette.yellow, palette.blue);
    centre_text (20, 60, 10, "Enter File Name");
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (25,12,30);
    state = getstr (25, 12, filename, hotkeys, 30);
    if ( state != K_ESC)
    {
        /* Is the entry a valid DOS filename  ? */

        fp = fopen ( filename, "wb");
        if ( fp == NULL)
            invalid_filename();
        else
        {
           fclose (fp);

           if ( strchr ( filename, '.') != NULL)
               invalid_filename();
           else
           {
               get_name_stub ( filename ); 
               sprintf ( filename, "%s.PR", exp_name );

               if ( file_exists ( filename ))
               {
                    if ( overwrite_pr_file (filename))
                       save_pr_file (filename);
               }
               else
                   save_pr_file ( filename);
           }
        }
    }

    restore_area (20, 9, 61, 15, save_buff);
    free ((char *)save_buff);
}


void get_printer_port ()
{
    char *port_buff;
    int ch;

    port_buff = (char *)malloc(PORTSIZE(48, 13, 69, 17));
    if ( port_buff == NULL)
        return;
    save_area (48, 13, 69, 17, port_buff);
    SetColourPair (palette.black, palette.green);
    draw_window (48, 13, 68, 16);
    WriteString ( 52, 14, "LPT1");
    WriteString ( 52, 15, "LPT2");
    WriteString ( 60, 14, "LPT3");
    WriteString ( 60, 15, "LPT4");
    SetColourPair (palette.red, palette.green);
    WriteChar (55, 14, '1'); WriteChar (55, 15, '2'); 
    WriteChar (63, 14, '3'); WriteChar (63, 15, '4');

    ch = get_key();
    while (( strchr ("1234", ch) == NULL) && ( ch != K_ESC))
        ch = get_key();
    switch ( ch )
    {
         case '1'   : print_options.printer_port = PRINTER_LPT1;
                      break;
         case '2'   : print_options.printer_port = PRINTER_LPT2;
                      break;
         case '3'   : print_options.printer_port = PRINTER_LPT3;
                      break;
         case '4'   : print_options.printer_port  = PRINTER_LPT4;
                      break;
         case K_ESC : break;
    }
    restore_area (48, 13, 69, 17, port_buff);
    free ((char *)port_buff);
}



