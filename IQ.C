#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <malloc.h>
#include <memory.h>
#include <graph.h>
#include <dos.h>
#include <string.h>
#include <bios.h>
#include <sys\types.h>
#include <time.h>
#include <sys\stat.h>

#include "errlist.h"
#include "palette.h"
#include "iq.h"
#include "keys.h"
#include "windows.h"
#include "config.h"
#include "menu.h"
#include "comm.h"
#include "break.h"
#include "iqcomms.h"
#include "register.h"



extern struct comms_def comms;
extern char mode_buffer[];
extern char info_block1[];
extern int data_transfer_mode;

extern void DrawFileViewer (char name[]);
extern void TextViewer (char name[]);
extern void UseViewer (char name[], unsigned long size);
extern int abandon_option_changes (void);
extern void show_error (int errnum);
extern void set_init_print_time_options(void );
extern int IQterminate (int type, int state);
extern void data_transfer(void);
extern int select_trans_mode ( void );
extern void push_wait ( void );
extern void simulate_push (int x, int y, char msg[], int width);
extern unsigned int draw_button (int x, int y, int width, char msg[], int state);
extern int wait_a_second ( void );
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern int serial_model (int model);
extern void flush_keyboard ( void );
extern void process_iq_models ( int model );
extern void process_zq_models ( int model );
extern void process_xe_models ( int model);
extern void show_model_name ( char name[]);
extern void process_el_models ( int model );
extern void write_blanks (int x, int y, int n);
extern trim_filename ( char s[], int max );
extern int get_yesno ( int x, int y, short fg, short bk );
extern void copy_check ( unsigned int );
extern void flush_keyboard ( void );
extern void comms_error (int err);
extern void WriteMultiChar (int x, int y, char ch, int n);
extern unsigned int get_key ( void );
extern void configure ( void );
extern void save_keyboard_flags(void);
extern void restore_keyboard_flags(void);
extern int select_file_to_print ( void );
extern int file_exists ( char s[]);
extern void do_serial_comms ( void );
extern int get_from_file_list(void);
extern void draw_menu (int x1, int y1, int x2, int y2, int m, int state);
extern void set_default_comms ( void );
extern void do_video_mode ( void );
extern void do_border_type ( void );
extern void do_shadow_type ( void );
extern void do_window_type ( void );
extern void do_viewer_type ( void );
extern void do_update_method ( void );
extern void do_print_file ( void );
extern void do_shell(void);
extern void do_view_file ( void );
extern void show_config_summary ( void );
extern void set_default_comms ( void );
extern void do_child_menu ( void );
extern void do_load_pr_settings ( void );
extern void do_save_pr_settings ( void );
extern void save_pr_file ( char name[]);
extern void load_pr_file (char name[]);

extern void do_printer_config ( void );
extern void dos_copy ( void );
extern void dos_rd ( void );
extern void dos_ren ( void );
extern void dos_md (void );
extern void dos_del ( void);
extern void do_print ( void );
extern void do_glossary ( void );
extern void print_file (char name[]);
extern int get_organiser_type (int model);

extern struct comms_def comms;
extern struct palette_str palette;
extern struct palette_str mono_mode_palette;
extern struct palette_str colour_mode_palette;
extern int request_directory ( void );
extern void break_down_mode_buffer ( void );
extern void create_dynamic_menu ( void );

int xx1[12]={188,144,147,150,145,223,168,150,147,156,144,135};
int xx2[10]={190,145,155,134,223,181,144,145,154,140};
int xx3[12]={183,144,136,158,141,155,223,172,147,150,145,145};
int  iq_type;
static int running = FALSE;
static char far *dosbusy;

#define BANNER_SIZE 5

char *banner[8]={
                 "YYYYYYY  YYYYYYY X     X XXXXXXX XXXXXXX XXXXXXX",
                 "   Y     Y     Y X     X X       X          X   ",
                 "   Y     Y  Y  Y X     X XXXXX   XXXXXXX    X   ",
                 "   Y     Y   Y Y X     X X             X    X   ",
                 "YYYYYYY  YYYYYYY XXXXXXX XXXXXXX XXXXXXX    X   "
                };

char file_list[MAX_FILE_LIST][80];
int file_list_count;
int take_a_look_at_the_password=FALSE;

struct print_time
{
    int           eol_terminator;
    int           throw_page;
    int           using;
    int           first,
                  last;
    unsigned char printer_port;
    int           header_mode;
} print_options;

struct dostime_t last_update_time;
short current_foreground,
      current_background;
char  gen_filename[255];
char  tmp[255];
char  *next_menu=NULL;

int  model=-1;
char current_file[255];          /* Name Of Last File Viewed/Printed */
char current_printer[30];       /* Name Of Active Printer */
char current_organiser[25];     /* Current Organiser Model */

#define ORGANISER_DEFINED      (strcmpi(current_organiser, "NONE") != 0)

unsigned char far *video_base;
int ccounter;

unsigned int cursor_type;
unsigned int cursor_position;

FILE         *config_file_ptr;
int          aborted, 
             finished;
int          ch;


extern int menu_option;
extern int current_menu;
extern int current_level;
extern int last_menu;
extern struct menu_def menus[MAX_MENUS];
extern int menu_table[MAX_MENUS][MAX_MENU_OPTIONS+1];
extern int level_option[MAX_MENU_LEVELS];
extern struct menu_def file_menu; 
extern unsigned char config_byte1,
                     config_byte2,
                     config_byte3;
extern unsigned char initial_config_byte1,
                     initial_config_byte2,
                     initial_config_byte3;

char *winbuffer1;
static struct SREGS d_sregs;
static union REGS d_regs;

extern void get_password (void);
extern void open_menu (int m, int layer);
extern void close_menu (int m, int layer);
extern void init_menus(void);
extern void shrink_menus ( int level );
extern void highlight_menu_bar (int menu, int state);
extern highlight_option (int m, int opt, int state);
extern void create_default_printer_file ( void );

void do_menu_tree(void);
void sn(void);
void init_video ( void );
void init_menus ( void );
void reset_video ( void );
void show_date ( void );            
void show_time ( void );

#ifndef DUMMY
void init_vars ( void );
#endif

void clear_time_box ( void );
void show_intro ( void );
void show_menu_bar (void);
void draw_basic_screen ( void );
void report_missing_config ( void );
void no_serial_comms ( char model[]);
void show_general_help ( void );
void show_help_main ( void );
int quit_to_dos ( void );
int really_save_changes ( void );
int load_config_file ( void );
void save_config_file ( void );
int warning ( void );
void data_transfer ( void );
void process_action ( int action );
void do_about ( void );
void centre_text ( int x1, int x2, int y, char msg[]);
void centre_push_button ( int x1, int x2, int y, char msg[]);

#ifndef DUMMY
int  do_copy_ccount ( unsigned int );
#endif

void do_date_format ( void );
void do_time_format ( void );
void do_child_menu( void );
void clear_date_box ( void );
int no_password_available ( char model[]);
void unsupported(char make[]);
int do_copy_ccount (unsigned int k);
void set_int_timer( void );
void reset_int_timer ( void );

#ifdef DUMMY
void say_dummy_version ( void );

void say_dummy_version ()
{
    char *buff;

    buff = (char *)malloc(PORTSIZE(15, 8, 66, 15));
    if (buff != NULL)
    {
        save_area (15, 8, 66, 15, buff);
        SetColourPair (palette.black, palette.green);
        draw_window (15, 8, 65, 14);
        window_title (15, 65, 8, "Trial Version", palette.red, palette.green);
        SetColourPair (palette.bright_white, palette.green);
        WriteString (17, 10, "This is a trial copy of IQuest and contains no");
        WriteString (17, 11, "decoding or data communications functionality.");
        centre_push_button (15, 65, 13, "&Ok");
        restore_area (15, 8, 66, 15, buff);
        free ((char *)buff );
    }
}
#else

void write_title ( void );

void write_title()
{
   int i,j;
   int x,y;

   x=17;
   y=7;
   
   for (i=0; i<BANNER_SIZE; i++)
   {   
      
      for (j=0; j<strlen(banner[i]); j++)   
      {
         SetColourPair(palette.black, palette.green);
         
         if (banner[i][j] == 'X')
         {   
             SetColourPair (palette.green, palette.black);
             WriteChar(x,y, ' ');
         }
         else
         {
            if (banner[i][j] == 'Y')
            {
               SetColourPair (palette.green, palette.red);
               WriteChar (x,y,' ');
            }
            else
               WriteChar (x,y, banner[i][j]);
         }
         x++;
      }
      y++;
      x=17;
   }
}
#endif

void main ( void );


void show_intro ()
{
  char *buf;
  char *buf2;

  buf = (char *)malloc(PORTSIZE(9, 6, 72, 19));
  buf2 = (char *)malloc(PORTSIZE(15, 13, 67, 18));
  if ((buf != NULL) && (buf2 != NULL))
  {
    
    save_area (9,6,72,19, buf);
    SetColourPair (palette.black, palette.green); 
    draw_window (9,6,70,18);
    draw_shadow (9,6,70,18);
    write_title();
    save_area(15,13,67,18,buf2);
    SetColourPair (palette.yellow, palette.blue);
    draw_window(15,13,65,16);
    draw_shadow(15,13,65,16);
    SetColourPair (palette.bright_white, palette.blue);
    WriteString (17, 14, "User Name : ");
    WriteString (17, 15, "Company   : ");
    SetColourPair (palette.bright_white, palette.black);
    write_blanks (29,14,35);
    write_blanks (29,15,35);
    WriteString (29, 14, USER_NAME);
    WriteString (29, 15, COMPANY_NAME);
    get_key();
    restore_area (15,13,67,18,buf2);
    restore_area (9,6,72,19,buf);
    free((char *)buf2);
    free((char *)buf);
  }
}

#ifndef DUMMY
int do_copy_ccount (unsigned int key)
{
   switch ( ccounter )
   {
      case 0 : if ((key == 'S') || (key == 's'))
                  return(1);       
                break;
      case 1 : if ((key == 'U') || (key == 'u'))
                  return(2);
               break;
      case 2 : if ((key == 'Z') || (key == 'z'))
                  return(3);
               break;
      case 3 : if ((key == 'I') || (key == 'i'))
                  return(4);
               break;
   }
   return(0);
}

#endif

void sn()
{
   char xxx[80];

   sprintf (xxx, " Serial Number : %s ", SERIAL_NO);
   window_title (3, 77, 20, xxx, palette.red, palette.cyan);
}

void clear_date_box ()
{
   SetColourPair (palette.bright_white, palette.black);
   WriteMultiChar (7, 22, 178, 35);
   WriteMultiChar (7, 23, 178, 35);
   WriteMultiChar (7, 24, 178, 35);
}

void get_month ( int month, char *mnth)
{
   if (config_byte2 & MSK_DATE_ABBREVIATION)
   {
      switch (month)
      {
         case 0 : strcpy (mnth, "01");   
                  break;
         case 1 : strcpy (mnth, "02");
                  break;
         case 2 : strcpy (mnth, "03");
                  break;
         case 3 : strcpy (mnth, "04");
                  break;
         case 4 : strcpy (mnth, "05");
                  break;
         case 5 : strcpy (mnth, "06");
                  break;
         case 6 : strcpy (mnth, "07");
                  break;
         case 7 : strcpy (mnth, "08");
                  break;
         case 8 : strcpy (mnth, "09");
                  break;
         case 9 : strcpy (mnth, "10");
                  break;
         case 10: strcpy (mnth, "11");
                  break;
         case 11: strcpy (mnth, "12");
                  break;
      }
   }
   else
   {
      switch (month)
      {  
         case 0 : strcpy (mnth, "January ");
                 break;
        case 1 : strcpy ( mnth, "February ");
                 break;
        case 2 : strcpy ( mnth, "March ");
                 break;
        case 3 : strcpy ( mnth, "April ");
                 break;
        case 4 : strcpy ( mnth, "May ");
                 break;
        case 5 : strcpy ( mnth, "June ");
                 break;
        case 6 : strcpy ( mnth, "July ");
                 break;
        case 7 : strcpy ( mnth, "August ");
                 break;
        case 8 : strcpy ( mnth, "September ");
                 break;
        case 9 : strcpy ( mnth, "October ");
                 break;
        case 10 : strcpy ( mnth, "November ");
                  break;
        case 11 : strcpy ( mnth, "December ");
                  break;
      }
   }   
}

void get_day ( int day, char weekday[] )
{
   if (config_byte2 & MSK_DATE_ABBREVIATION)
      strcpy (weekday, "");
   else
   {
     switch (day)
     {
        case 0 : strcpy (weekday, "Sunday ");
                 break;
        case 1 : strcpy (weekday, "Monday ");
                 break;
        case 2 : strcpy (weekday, "Tuesday ");
                 break;
        case 3 : strcpy (weekday, "Wednesday ");
                 break; 
        case 4 : strcpy (weekday, "Thursday ");
                 break;
        case 5 : strcpy (weekday, "Friday ");
                 break;
        case 6 : strcpy (weekday, "Saturday ");
                 break;
     }
   }
}

void get_extension ( int day_number, char *ext)
{
   if (config_byte2 & MSK_DATE_ABBREVIATION)
      strcpy (ext, "");
   else
   {
     switch (day_number)
     {
       case 2 :
       case 22 :  strcpy (ext, "nd ");
                  break;
       case 3 :
       case 23 : strcpy (ext, "rd ");
                 break;
       case 1 :
       case 21 :
       case 31 : strcpy ( ext, "st ");
                 break;
       default : strcpy ( ext, "th ");
     }
   }
}

void show_date ()
{
   struct dosdate_t date;
   char month_name[15];
   char weekday[15];
   char ext[4];
   char year[5];
   char x[3];

   clear_date_box();
   _dos_getdate (&date);
   get_month (date.month-1, month_name);
   get_day (date.dayofweek, weekday);
   get_extension ( date.day, ext);
   if (config_byte2 & MSK_DATE_ABBREVIATION)
      sprintf (year, "%d", date.year-1900);
   else
      sprintf (year, "%d ", date.year);
   sprintf (x, "%d", date.day);
   switch ( (config_byte2 & MSK_DATE_FORMAT) >> 1)
   {
       case 0 : if (config_byte2 & MSK_DATE_ABBREVIATION)
                    sprintf (tmp, "%s/%s/%s", x, month_name, year);
                else
                {
                   if (config_byte2 & MSK_DAY_NAME)
                   {
                      strcpy (tmp, weekday);
                      strcat (tmp, x);
                   }
                   else
                     strcpy (tmp, x);
                   strcat (tmp, ext);
                   strcat (tmp, month_name);
                   strcat (tmp, year);
                }
                break;
       case 1 : if (config_byte2 & MSK_DATE_ABBREVIATION)
                   sprintf (tmp, "%s/%s/%s", month_name, x, year);
                else
                {
                   strcpy (tmp, month_name);
                   strcat (tmp, x);
                   strcat (tmp, ext);
                   strcat (tmp, year);
                   if (config_byte2 & MSK_DAY_NAME)
                      strcat (tmp, weekday);
                }
                break;
       case 2 : if (config_byte2 & MSK_DATE_ABBREVIATION)
                    sprintf (tmp, "%s/%s/%s", year, month_name, x);
                else
                {
                    strcpy (tmp, year);
                    strcat (tmp, month_name);
                    strcat (tmp, x);
                    strcat (tmp, ext);
                    if ( config_byte2 & MSK_DAY_NAME)
                        strcat (tmp, weekday);
                }
                break;
       case 3 : if (config_byte2 & MSK_DATE_ABBREVIATION)
                   sprintf (tmp, "%s/%s/%s", year, x, month_name);
                else
                {
                    strcpy (tmp, year);
                    strcat (tmp, x);
                    strcat (tmp, ext);
                    strcat (tmp, month_name);
                    if (config_byte2 & MSK_DAY_NAME)
                        strcat (tmp, weekday);
                }
                break;
       case 4 : if (config_byte2 & MSK_DATE_ABBREVIATION)
                   sprintf (tmp, "%s/%s/%s", x, year, month_name);
                else
                {
                    strcpy (tmp, x);
                    strcat (tmp, ext);
                    strcat (tmp, year);
                    strcat (tmp, month_name);
                    if (config_byte2 & MSK_DAY_NAME)
                       strcat (tmp, weekday);
                }
                break;
       case 5 : if (config_byte2 & MSK_DATE_ABBREVIATION)
                   sprintf (tmp, "%s/%s/%s", month_name, year, x);
                else
                {
                   strcpy (tmp, month_name);
                   strcat (tmp, year);
                   strcat (tmp, x);
                   strcat (tmp, ext);
                   if (config_byte2 & MSK_DAY_NAME)
                      strcat (tmp, weekday);
                }
                break;
   }
   SetColourPair (palette.bright_white, palette.red);      
   draw_box (7, 22, 7+strlen(tmp)+3, 24);
   SetColourPair (palette.yellow, palette.red);
   WriteString (9, 23, tmp);
}

void clear_time_box ()
{
   SetColourPair (palette.bright_white, palette.black);
   WriteMultiChar (55, 22, 178, 23);
   WriteMultiChar (55, 23, 178, 23);
   WriteMultiChar (55, 24, 178, 23);
}

void show_time ()
{
   struct dostime_t time;
   
   _dos_gettime (&time);
   clear_time_box();
   if (config_byte2 & MSK_TIME_FORMAT)
   {
      if (config_byte2 & MSK_TIME_FRAME)
         sprintf (tmp, "%02d:%02d:%02d", time.hour, time.minute, time.second);
      else
      {
         if (time.hour > 12)
         {
            sprintf (tmp, "%02d:%02d:%02d", time.hour-12, time.minute, time.second);
            strcat (tmp, "pm");
         }
         else
         {
            sprintf (tmp, "%02d:%02d:%02d", time.hour, time.minute, time.second);
            strcat (tmp, "am");
         }
      }
   }
   else
   {
      if (config_byte2 & MSK_TIME_FRAME)
         sprintf (tmp, "%02d:%02d", time.hour, time.minute);
      else
      {
         if (time.hour > 12)
         {
            sprintf (tmp, "%02d:%02d", time.hour-12, time.minute);
            strcat (tmp, "pm");
         }
         else
         {
            sprintf (tmp, "%02d:%02d", time.hour, time.minute);
            strcat (tmp, "am");
         }
      }
   }
   SetColourPair (palette.bright_white, palette.red);
   draw_box (70-strlen(tmp), 22, 74, 24);
   SetColourPair (palette.yellow, palette.red);
   WriteString (70-strlen(tmp)+2, 23, tmp);
   _dos_gettime (&last_update_time);
}

void show_menu_bar ()
{
   SetColourPair (palette.black, palette.white);
   write_blanks (1,1, 80);
   WriteString (1,1, "File");    
   WriteString (9, 1, "Configure");     
   WriteString (23,1, "Model");                                            
   WriteString (33, 1, "Process");
   WriteString (43, 1, "Exit");
   WriteString (67,1, "Help");
   SetColourPair (palette.red, palette.white);
   WriteChar (1,1, 'F');
   WriteChar (9,1, 'C');
   WriteChar (23,1, 'M');
#ifndef DUMMY
   if (ORGANISER_DEFINED)
     WriteChar (33, 1, 'P');
#endif
   WriteChar (43, 1, 'E');
   WriteChar (67,1, 'H');
}

void do_about ()
{
   char *b;
   char x[80];


   b = (char *)malloc (PORTSIZE(20, 6, 61, 18));
   if (b != NULL)
   {
      save_area (20, 6, 61, 18, b);
      SetColourPair (palette.yellow, palette.green);
      draw_window (20, 6, 60, 17);
      SetColourPair (palette.red, palette.green);
      sprintf (x, "IQuest %s", RM_VERSION);
      WriteString (34, 8, x);
      SetColourPair (palette.black, palette.green);
      WriteString (25, 9, "Organiser Interrogation Software"); 
      centre_text (20, 60, 11, "(c) IQuest Software 1996"); 
      centre_text (20, 60, 12, "All Rights Reserved.");
      SetColourPair (palette.bright_white, palette.green);
#ifdef DUMMY
     strcpy (x, "** TRIAL VERSION **");
#else
      sprintf (x, "( Last Updated %s )", __DATE__);
#endif
      centre_text (20, 60, 14, x);
      centre_push_button (20, 60, 16, "&Continue");
      restore_area(20, 6, 61, 18, b);
      free ((char *)b);
   }
}

void draw_basic_screen ()
{
    int i;

    SetColourPair (palette.bright_white, palette.black);
    for (i=1; i <= 25; i++)
      WriteMultiChar (1, i, 178, 80);
    hide_cursor();
    SetColourPair (palette.black, palette.cyan);
    draw_box (3, 4, 77, 20); 
    
    show_model_name ( current_organiser );
    SetColourPair (palette.bright_white, palette.red);
    show_menu_bar (); 
    show_date ();
    show_time(); 
    sn();

#ifndef DUMMY
    show_intro();
#endif

    show_model_name ( current_organiser );  
}

int load_config_file ()
{
    int retval;
    int i;

    hide_cursor();
    retval = ERR_OK;

    if ( file_exists ( CONFIG_FILENAME ) )
    {
        config_file_ptr = fopen ( CONFIG_FILENAME, "rb");
        fscanf (config_file_ptr, "%c%c%c", &config_byte1, &config_byte2, &config_byte3);
        fread ( &comms.port, sizeof (struct comms_def), 1, config_file_ptr);
        fscanf ( config_file_ptr, "%s\n\r", current_organiser);
        fscanf ( config_file_ptr, "%d\n\r", &model);
        fscanf ( config_file_ptr, "%d\n\r", &iq_type);
        fscanf ( config_file_ptr, "%d\n\r", &file_list_count);
        for (i=0; i<file_list_count; i++)
          fscanf (config_file_ptr, "%s\n\r", file_list[i]);
        
        for (i=0; i < strlen(current_organiser); i++)
           if (current_organiser[i] == '_') 
              current_organiser[i] = ' ';
        
        fclose (config_file_ptr);
    }
    else
    {
        config_byte1 = DEFAULT_CONFIG_BYTE1;
        config_byte2 = DEFAULT_CONFIG_BYTE2;
        config_byte3 = DEFAULT_CONFIG_BYTE3;
        strcpy (current_organiser, "NONE");
        iq_type=-1;
        model = -1;
        set_default_comms();
        retval = ERR_NO_FILE;
        file_list_count=0;
    }
    initial_config_byte1 = config_byte1;
    initial_config_byte2 = config_byte2;
    initial_config_byte3 = config_byte3;
    return (retval);
}

void report_missing_config()
{
    char *errfile;
    char str[255];

    errfile = (char *)malloc (PORTSIZE(22, 9, 59, 13));
    if (errfile == NULL)
        return;
    save_area (22,9,59,13,errfile);
    SetColourPair (palette.bright_white, palette.red);
    draw_window (22,9,58,12);
    SetColourPair (palette.yellow, palette.red);
    sprintf (str, "Cannot Find '%s'", CONFIG_FILENAME);
    WriteString ( 22+((37 - strlen (str))/2), 10, str);
    WriteString ( 27, 11, "Using Default Configuration");
    get_key();
    restore_area (22,9,59,13,errfile);
    free ((char *)errfile);
}

void save_config_file ()
{
    int i;

    config_file_ptr = fopen ( CONFIG_FILENAME, "wb");
    if (config_file_ptr != NULL)
    {
       fprintf ( config_file_ptr, "%c%c%c", config_byte1, config_byte2, config_byte3);
       fwrite (&comms.port, sizeof (struct comms_def), 1, config_file_ptr);
       for (i=0; i < strlen(current_organiser); i++)
          if (current_organiser[i] == ' ')
             current_organiser[i] = '_';
       fprintf ( config_file_ptr, "%s\n\r", current_organiser);
       fprintf ( config_file_ptr, "%d\n\r", model);
       fprintf (config_file_ptr, "%d\n\r", iq_type);
       fprintf (config_file_ptr, "%d\n\r", file_list_count);
       for (i=0; i<file_list_count;i++)
          fprintf (config_file_ptr, "%s\n\r", file_list[i]);
       fclose (config_file_ptr);
    }
}

void show_general_help ()
{
    hide_cursor();
    SetColourPair (palette.black, palette.green);
    draw_window ( 10, 5, 67, 16);
    ShowHeader ( 10, 67, 6, "General Help");
    WriteString ( 17,  8, "ALT-F      - File Processing.");
    WriteString ( 17,  9, "ALT-C      - Program Configuration.");
    WriteString ( 17, 10, "ALT-M      - Select Organiser Make/Model."); 
    WriteString ( 17, 11, "ALT-H      - General Program Help.");
    WriteString ( 17, 13, "ALT-E      - Leave IQuest.");
    centre_push_button (10, 67, 15, "&Ok");
    flush_keyboard();
}

void show_help_main ()
{
    winbuffer1 = (char *)malloc(PORTSIZE(10, 5, 71, 17));
    if (winbuffer1 != NULL)
    {
        save_area ( 10, 5, 71, 17, winbuffer1);
        show_general_help();
        restore_area ( 10, 5, 71,17, winbuffer1);
        free ((char *)winbuffer1);
    }
}

void init_video ()
{
    video_base = (unsigned char far *)0xB8000000;

    if ( ( config_byte1 & MSK_VIDEO_DETECT) == MSK_VIDEO_DETECT )
    {
        if ( _setvideomode (_TEXTC80) == 0) /* give priority to CGA then MDA */
        {
            _setvideomode (_TEXTMONO);
            video_base = (unsigned char far *)0xB0000000;
            memcpy (&palette.black, &mono_mode_palette.black, sizeof (struct palette_str));
        }
        else
            memcpy (&palette.black, &colour_mode_palette.black, sizeof (struct palette_str));
    }
    else
    {
        /* MONG MODE PROTECTION :

           Ensure that if configured for colour, but CGA mode is not
           present then drop back to mono - well it may happen i suppose

        */

        if ( ( config_byte1 & MSK_VIDEO_COLOUR) == MSK_VIDEO_COLOUR)
        {
            if ( _setvideomode (_TEXTC80) != 0) /* Mode available */
              memcpy (&palette.black, &colour_mode_palette.black, sizeof (struct palette_str));
            else
            {
                _setvideomode (_TEXTMONO);
                video_base = (unsigned char far *)0xB0000000;
                memcpy (&palette.black, &mono_mode_palette, sizeof (struct palette_str));
            }
        }
        else
        {
            _setvideomode (_TEXTMONO);
            video_base = (unsigned char far *)0xB0000000;
            memcpy (&palette.black, &mono_mode_palette.black, sizeof (struct palette_str));
        }
    }
    draw_basic_screen(); 
}

void reset_video ()
{
   _setvideomode (_DEFAULTMODE);
}

int really_save_changes ()
{
    char *save_buff;
    int  save_data;

    save_buff = (char *)malloc (PORTSIZE(26, 9, 55, 13));
    if ( save_buff != NULL)
    {
      save_area (26,9,55,13,save_buff);
      SetColourPair (palette.yellow, palette.red);
      draw_window (26,9,54, 12);
      SetColourPair (palette.bright_white, palette.red);
      WriteString (34, 10, "Save Changes ?");
      save_data = get_yesno (34, 11, palette.white, palette.red);
      restore_area (26, 9, 55, 13, save_buff);
      free ((char *)save_buff);
    }
    else
      save_data = TRUE;
    return (save_data);
}

int quit_to_dos()
{
    char *quit_buff;
    int   goto_dos;

    quit_buff = (char *)malloc(PORTSIZE(28, 9, 53, 13));
    if ( quit_buff == NULL)
        return (TRUE);
    save_area (28, 9, 53, 13, quit_buff);
    SetColourPair (palette.bright_white, palette.blue);
    draw_window (28, 9, 52, 12);
    SetColourPair (palette.yellow, palette.blue);
    WriteString ( 34, 10, "Exit To DOS ?");
    goto_dos = get_yesno (34, 11, palette.yellow, palette.blue);
    restore_area(28, 9, 53, 13, quit_buff);
    free ((char *)quit_buff);
    return ( goto_dos );
}

int warning ()
{
   char *warning;
   int answer;

#ifdef DUMMY
   return (TRUE);
#endif

   answer = FALSE;
   warning = (char *)malloc(PORTSIZE(10, 7, 71, 18));
   if (warning != NULL)
   {
     save_area (10, 7, 71, 18, warning);
     SetColourPair (palette.bright_white, palette.red);
     draw_window (10, 7, 70, 17);
     SetColourPair (palette.yellow, palette.red);
     centre_text (10, 70,  8, "** WARNING **");
     centre_text (10, 70,  10, "THIS SOFTWARE IS FOR USE BY LAW ENFORCEMENT");
     centre_text (10, 70,  11, "BODIES ONLY. UNAUTHORISED USE OF THIS PRODUCT");
     centre_text (10, 70,  12, "TO ACQUIRE PRIVATE INFORMATION IS A CRIMINAL OFFENCE.");
     centre_text (10, 70, 14, "DO YOU WISH TO PROCEED ?");
     answer = get_yesno (34, 15, palette.yellow, palette.red);
     restore_area (10, 7, 71, 18, warning);
     free ((char *)warning);
   }
   return (answer);
}

#ifndef DUMMY
void init_vars ()
{
    char         *buff;
    int          i;
    unsigned int key;
    int          finish;

    ccounter = 0;
    buff = (char *)malloc(PORTSIZE(20, 8, 61, 17));
    if (buff != NULL)
    {
       save_area (20,8,61,17,buff);
       SetColourPair (palette.black, palette.green);
       draw_window (20,8,60,16);
       SetColourPair (palette.red, palette.green);
       centre_text (20, 60, 9, "IQuest v1.1");
       SetColourPair (palette.black, palette.green);
       WriteString (22, 11, "Coding & Design        : ");
       WriteString (22, 12, "Design & Documentation : ");
       WriteString (22, 13, "Additional Information : ");
       SetColourPair (palette.bright_white, palette.green);
       for (i=0; i < 12; i++)
         WriteChar (47+i, 11, (char)xx1[i]);
       for (i=0; i < 10; i++)
         WriteChar (47+i, 12, (char)xx2[i]);
       for (i=0; i< 12; i++)
         WriteChar (47+i, 13, (char)xx3[i]);
       draw_button (36, 15, 10, "&Continue", BUTTON_UP);
       finish = FALSE;
       while (! finish)
       {
          key = get_key();
          switch (key)
          {
             case K_CR :
             case 'c'  :
             case 'C'  : simulate_push (36, 15, "&Continue", 10);
                         finish = TRUE;
                         push_wait();
                         break;
             case '#'  : SetColourPair (palette.bright_white, palette.green);
                         for (i=0; i < 12; i++)
                           WriteChar (47+i, 11, (char)(xx1[i] ^ 255));
                         for (i=0; i < 10; i++)
                           WriteChar (47+i, 12, (char)(xx2[i] ^ 255));
                         for (i=0; i < 12; i++)
                            WriteChar (47+i, 13, (char)(xx3[i] ^ 255));
                         break;
             default   : break;
          }
       }
       restore_area (20,8,61,17, buff);
       free((char *)buff);
    }
}
#endif

void no_serial_comms (char model[])
{
   char *buff;
   char str[80];

   buff = (char *)malloc(PORTSIZE(20, 8, 61, 14));
   if (buff != NULL)
   {
      save_area (20,8,61,14,buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window (20,8,60,13);
      SetColourPair (palette.yellow, palette.red);
      sprintf (str, "Sorry, The %s Does Not", model);
      centre_text (21, 60, 9, str);
      centre_text (21,60,10,"Support Serial Comms");
      centre_push_button (20,60,12, "&Ok");
      restore_area (20,8,61,14,buff);
      free((char *)buff);
   }
}




void unsupported ( char name[])
{
   char *buff;

    buff = (char *)malloc(PORTSIZE(20, 9, 61, 16));
    if (buff != NULL) 
    {
       save_area (20, 9, 61, 16, buff);
       SetColourPair (palette.bright_white, palette.blue);
       draw_window (20, 9, 60, 15);
       window_title (20, 60, 9, name, palette.red, palette.blue);
       SetColourPair (palette.yellow, palette.blue);
       centre_text (20, 60, 11, "Sorry, It Is Not Possible To Remove");
       centre_text (20, 60, 12, "The Password On This Organiser Model");
       centre_push_button (20, 60, 14, "&Continue");
       restore_area (20, 9, 61, 16, buff);
       free ((char *)buff);
    }
}

int no_password_available ( char make[])
{
   int 
       bad_el,
       bad_zq,
       bad_xe;

   bad_el = 
           ((strcmpi (make, "Sharp EL-6190") == 0) ||
            (strcmpi (make, "Sharp EL-6000") == 0) ||
            (strcmpi (make, "Sharp EL-6170") == 0) ||
            (strcmpi (make, "Sharp EL-6330") == 0) ||
            (strcmpi (make, "Sharp EL-6360") == 0) ||
            (strcmpi (make, "Sharp EL-6390") == 0) ||
            (strcmpi (make, "Sharp EL-9000") == 0) ||
            (strcmpi (make, "Sharp EL-9000 MK2") == 0) ||
            (strcmpi (make, "Sharp EL-9200") == 0) ||
            (strcmpi (make, "Sharp EL-9200 MK2") == 0)
           );
   
   bad_zq = 
           ((strcmpi (make, "Sharp ZQ-1000") == 0) ||
            (strcmpi (make, "Sharp ZQ-1050") == 0) ||
            (strcmpi (make, "Sharp ZQ-1200") == 0) ||
            (strcmpi (make, "Sharp ZQ-1250A") == 0) ||
            (strcmpi (make, "Sharp ZQ-2200") == 0) ||
            (strcmpi (make, "Sharp ZQ-2400") == 0) ||
            (strcmpi (make, "Sharp ZQ-2450") == 0) ||
            (strcmpi (make, "Sharp ZQ-2500") == 0) ||
            (strcmpi (make, "Sharp ZQ-2700") == 0) ||
            (strcmpi (make, "Sharp ZQ-2750") == 0) ||
            (strcmpi (make, "Sharp ZQ-3000") == 0) ||
            (strcmpi (make, "Sharp ZQ-3200") == 0) ||
            (strcmpi (make, "Sharp ZQ-3250") == 0) ||
            (strcmpi (make, "Sharp ZQ-4450") == 0) ||
            (strcmpi (make, "Sharp ZQ-5000") == 0) ||
            (strcmpi (make, "Sharp ZQ-5100M") == 0) ||
            (strcmpi (make, "Sharp ZQ-6000") == 0) ||
            (strcmpi (make, "Sharp ZQ-6100M") == 0) ||
            (strcmpi (make, "Sharp ZQ-6200") == 0) ||
            (strcmpi (make, "Sharp ZQ-6300M") == 0)
          );

   bad_xe = (strcmpi (make, "Sharp Xeraus") == 0);
   
   return ( bad_zq || bad_xe || bad_el);

}

void process_action ( int action )
{
  int x1,y1,x2,y2;
  char *vwr_buff;
  FILE *vwr_file;
  struct find_t c_file;

  highlight_option (current_menu, menu_option, TRUE);
  take_a_look_at_the_password=FALSE;
  switch ( action )
  {
     case MENU_TREE    : do_menu_tree();
                         break;
     case DO_GLOSSARY  : do_glossary ();
                         break;
     case DO_PRINT     : if (!select_file_to_print ())
                            print_file (current_file);
                         break;
     case DO_REVIEW_PRINT : do_print ();
                            break;
     case VIEW_FILE    : do_view_file ();
                         if (file_list_count > 0)
                         {
                                menus[FILE_MENU].attrib[4] = MENU_NORMAL;
                                menu_table[FILE_MENU][5] = PREVIOUS_FILES;
                                x1 = menus[FILE_MENU].start_x;
                                y1 = menus[FILE_MENU].start_y;
                                x2 = menus[FILE_MENU].start_x+menus[FILE_MENU].width+2;
                                y2 = menus[FILE_MENU].start_y+menus[FILE_MENU].number_options+1;
                                draw_menu(x1,y1,x2,y2, FILE_MENU, TRUE);
                         }
                         break;
     case ABOUT_IQUEST   : do_about ();
                           break;
     case LEAVE_IQUEST   : finished = quit_to_dos();
                           break;
     case VIDEO_MODE     : do_video_mode ();
                           shrink_menus ( current_level );
                           break;
     case DOS_SHELL      : do_shell();
                           break;
     case BORDER_TYPE    : do_border_type ();
                           break;
     case SHADOW_TYPE    : do_shadow_type ();
                           break;
     case WINDOW_TYPE    : do_window_type ();
                           break;
     case SCREEN_UPDATE_METHOD : do_update_method();
                                 break;
     case FILE_VIEWER    : do_viewer_type ();
                           break;
     case CONFIG_SUMMARY : show_config_summary ();
                           break;
     case TIME_FORMAT    : do_time_format ();
                           break;
     case DATE_FORMAT    : do_date_format ();
                           break;
     case IQ_7000      :
     case IQ_7200      :
     case IQ_7400      :
     case IQ_7600      : 
     case IQ_7100M     : 
     case IQ_7300M     :
     case IQ_7420      :
     case IQ_7620      :
     case IQ_7690      : model = action;
                         iq_type =  IQ7000;
                         process_iq_models(action);
                         break;
     case IQ_8000      :
     case IQ_8200      :
     case IQ_8400      :
     case IQ_8100M     :
     case IQ_8300M     :
     case IQ_8500M     :
     case IQ_8600M     : model = action;
                         iq_type = IQ8000;
                         process_iq_models ( action );
                         break;
     case IQ_8900      :
     case IQ_8920      : 
     case IQ_9000      :
     case IQ_9000MK2   :
     case IQ_9200      :
     case IQ_9200MK2   : model = action;
                         iq_type = IQ9000;
                         process_iq_models ( action );
                         break;
     case XE_7000      : model = action;
                         iq_type = -1;
                         process_xe_models ( action);
                         break;
     case EL_6320      : 
     case EL_6000      :
     case EL_6170      :
     case EL_6190      :
     case EL_6330      :
     case EL_6360      :
     case EL_6390      : model = action;
                         iq_type = -1;
                         process_el_models ( action );
                         break;
     case ZQ_1000      :
     case ZQ_1050      :
     case ZQ_1200      :
     case ZQ_1250A     :
     case ZQ_2200      :
     case ZQ_2400      :
     case ZQ_2450      :
     case ZQ_2500      :
     case ZQ_2700      :
     case ZQ_2750      :
     case ZQ_3000      :
     case ZQ_3200      :
     case ZQ_3250      :
     case ZQ_4450      :
     case ZQ_5000      :
     case ZQ_5100M     :
     case ZQ_6000      :
     case ZQ_6100M     :
     case ZQ_6200      :
     case ZQ_6300M     : model = action;
                         iq_type = -1;
                         process_zq_models ( action);
                         break;
     case ZQ_2250      :
     case ZQ_5200      :
     case ZQ_5300      : 
     case ZQ_5300M     : model = action;
                         iq_type = IQ7000;
                         process_zq_models( action );
                         break;
     case PRINTER_LOAD : do_load_pr_settings ();
                         break;
     case PRINTER_SAVE : do_save_pr_settings ();
                         break;
     case PRINTER_EDIT : do_printer_config ();
                         break;
     case DOS_DEL      : dos_del();
                         break; 
     case DOS_COPY     : dos_copy();
                         break; 
     case DOS_REN      : dos_ren();
                         break; 
     case DOS_MD       : dos_md();
                         break;            
     case DOS_RD       : dos_rd();
                         break;    
#ifdef DUMMY
     case GET_DATA      :
     case GET_PASSWORD :
                         break;
#else
     case GET_DATA     : data_transfer();
                         break;
     case GET_PASSWORD : if ( no_password_available (current_organiser) )
                            unsupported( current_organiser);
                         else
                            take_a_look_at_the_password=TRUE;
#endif
                         break;
     case SELECT_COM1  : comms.port = PORT1;
                         shrink_menus(current_level);
                         show_model_name (current_organiser);
                         break;
     case SELECT_COM2  : comms.port = PORT2;
                         shrink_menus(current_level);
                         show_model_name (current_organiser);
                         break;
     case SELECT_COM3  : comms.port = PORT3;
                         shrink_menus(current_level);
                         show_model_name (current_organiser);
                         break;
     case SELECT_COM4  : comms.port = PORT4;
                         shrink_menus(current_level);
                         show_model_name (current_organiser);
                         break;
     case TRANS_MODE   : data_transfer_mode = select_trans_mode();
                         break;
     case PREVIOUS_FILES : if (get_from_file_list())
                           {
                               if (config_byte1 & MSK_FILE_VIEWER) /* Hex Viewer */
                               {
                                     _dos_findfirst (current_file, _A_NORMAL, &c_file);
                                     vwr_buff = (char *)malloc (PORTSIZE(1,1,80,23));
                                     if (vwr_buff != NULL)
                                     {
                                          save_area (1,1,80,23,vwr_buff);
                                          DrawFileViewer ( current_file);
                                          vwr_file = fopen ( current_file, "rb");
                                          UseViewer ( c_file.name, c_file.size);
                                          fclose (vwr_file);
                                          restore_area (1,1,80,23,vwr_buff);
                                          free ((char *)vwr_buff);
                                     }
                               }
                               else
                                  TextViewer ( current_file ); /* ASCII Viewer */
                           }
                          break;
     default           :
                         break;
  }
}

void show_expand_options(int state)
{
   SetColourPair (palette.bright_white, palette.green);
   WriteChar (29, 10, ' ');
   if (state)
     WriteChar (29, 10, 'X');
}

void do_menu_tree ()
{
   char *buff;
   unsigned char old_config_byte1;
   unsigned int key;
   int          aborted;
   int          state;

   buff=(char *)malloc(PORTSIZE(25,8,56,15));
   if (buff !=NULL)
   {
      save_area(25,8,56,15,buff);
      SetColourPair (palette.black, palette.green);
      draw_window(25,8,55,13);
      window_title(25,55,8,"Expand Menus", palette.red, palette.green);
      SetColourPair (palette.black, palette.green);
      WriteString (27, 10, "[   ] Display Child Menu");
      SetColourPair (palette.red, palette.green);
      WriteChar (33,10, 'D');
      aborted=FALSE;
      old_config_byte1 = config_byte1;
      state = config_byte1 & MSK_MENU_TREE;
      draw_button (30, 12, 4, "&Ok", BUTTON_UP);
      draw_button (43, 12, 8, "&Cancel", BUTTON_UP);
      while(!aborted)
      {
         config_byte1 &= ~MSK_MENU_TREE;
         if (state)
            config_byte1 |= MSK_MENU_TREE;

         show_expand_options(state);
         key = get_key();
         switch (key )
         {
            case ' ' :
            case 'D' :
            case 'd' : state = !state;
                       break;
            case 'O'  :
            case 'o'  : 
            case K_CR :aborted = TRUE;
                       simulate_push(30,12, "&Ok", 4);
                       break;
            case K_ESC:
            case 'C'  :
            case 'c'  : aborted = TRUE;
                        simulate_push(43, 12, "&Cancel", 8);
                        if (old_config_byte1 != config_byte1)
                        {
                           if (abandon_option_changes())
                              config_byte1 = old_config_byte1;
                        }
                        break;
            default    : break;
         }
      }
      restore_area(25,8,56,15,buff);
      free((char *)buff);
   }

}

void main ()
{
    unsigned int key;
    int          status;

    aborted = FALSE;
    finished = FALSE;

    ctrl_c (INTERCEPT);
    file_list_count=0;

    status = load_config_file();  
    init_video();
    if (warning())
    {
       init_menus();
       if (status != ERR_OK)
          report_missing_config();  
                           
       if (! file_exists (DEFAULT_PR_FILE))
          create_default_printer_file();

       load_pr_file (DEFAULT_PR_FILE);
       set_init_print_time_options();

       set_default_comms ();
       
#ifdef DUMMY
        say_dummy_version();
#endif

       while (! finished)
       {

          if (!kbhit ())
          {
             if (wait_a_second())
                show_time();
          }
          else
          {
             key = get_key();
#ifndef DUMMY
            if ((ccounter = do_copy_ccount(key)) == 4)
               init_vars();
#endif
            switch ( key )
            {
               case K_ESC  :   if (current_menu != -1) 
                               {
                                  highlight_menu_bar (current_menu, FALSE);
                                  close_menu (current_menu, current_level);
                                  if (menu_table[current_menu][0] != -999)
                                  {
                                    current_level--;
                                    current_menu = menu_table[current_menu][0];
                                    draw_menu ( 
                                               menus[current_menu].start_x,
                                               menus[current_menu].start_y,
                                               menus[current_menu].start_x + menus[current_menu].width+2,
                                               menus[current_menu].start_y + menus[current_menu].number_options+1,
                                               current_menu,
                                               TRUE
                                              ); 
                                    menu_option = level_option[current_level];        
                                    highlight_option (current_menu, menu_option, TRUE);
                                  
                                  }
                                  else
                                  {
                                     current_menu = -1;
                                     menu_option = -1;
                                  }
                                }
                                break;
              case K_CR       : if (current_menu != -1)
                                {
                                   if (menus[current_menu].attrib[menu_option] == MENU_HAS_SUBS)
                                   {
                                      level_option[current_level] = menu_option;
                                      current_level++;
                                      draw_menu ( 
                                                 menus[current_menu].start_x,
                                                 menus[current_menu].start_y,
                                                 menus[current_menu].start_x + menus[current_menu].width+2,
                                                 menus[current_menu].start_y + menus[current_menu].number_options+1,
                                                 current_menu,
                                                 FALSE
                                                ); 
                                      current_menu = menu_table[current_menu][menu_option+1];
                                      menu_option = 0;
                                      open_menu (current_menu, current_level);
                                   }
                                   else
                                   {   
                                      draw_menu ( 
                                                 menus[current_menu].start_x,
                                                 menus[current_menu].start_y,
                                                 menus[current_menu].start_x + menus[current_menu].width+2,
                                                 menus[current_menu].start_y + menus[current_menu].number_options+1,
                                                 current_menu,
                                                 FALSE
                                                ); 
                                      process_action ( menu_table[current_menu][menu_option+1]);
                                      if (current_menu != -1)
                                      {   
                                          if (take_a_look_at_the_password)
                                          {     
                                              close_menu (current_menu, current_level);
                                              current_menu=-1;
                                              get_password();
                                          }
                                          else
                                          { 
                                              draw_menu ( 
                                                         menus[current_menu].start_x,
                                                         menus[current_menu].start_y,
                                                         menus[current_menu].start_x + menus[current_menu].width+2,
                                                         menus[current_menu].start_y + menus[current_menu].number_options+1,
                                                         current_menu,
                                                         TRUE
                                                        ); 
                                              highlight_option (current_menu, menu_option, TRUE);
                                          } 
                                      }
                                   }
                                }
                                break;
              case K_CSR_UP   : if (current_menu != -1)
                                {
                                    highlight_option (current_menu, menu_option, FALSE);
                                    menu_option--;
                                    if (menu_option < 0 )
                                       menu_option = menus[current_menu].number_options - 1;
                                    while ((menus[current_menu].attrib[menu_option] == MENU_SEPARATOR) ||
                                           (menus[current_menu].attrib[menu_option] == MENU_GREYED)
                                          )
                                    {       
                                       menu_option--;
                                       if (menu_option < 0)
                                         menu_option = menus[current_menu].number_options - 1;
                                    }
                                    highlight_option (current_menu, menu_option, TRUE);
                           
                                }     
                                break;
              case K_CSR_DN   : if (current_menu != -1)
                                {
                                   highlight_option (current_menu, menu_option, FALSE);
                                   menu_option++;
                                   if (menu_option == menus[current_menu].number_options)
                                     menu_option = 0;
                                   while((menus[current_menu].attrib[menu_option] == MENU_SEPARATOR) ||
                                         (menus[current_menu].attrib[menu_option] == MENU_GREYED)
                                        )
                                   {     
                                       menu_option++;
                                       if (menu_option == menus[current_menu].number_options)
                                         menu_option = 0;
                                   }
                                   highlight_option ( current_menu, menu_option, TRUE);
                                }
                                break;
                     
              case K_CSR_RT   : if (current_menu != -1)
                                {   
                                   if (menu_table[current_menu][0] == -999) 
                                   {
                                     if ((menus[current_menu].attrib[menu_option] == MENU_HAS_SUBS) &&
                                         (config_byte1 & MSK_MENU_TREE)
                                         )
                                      {
                                         level_option[current_level] = menu_option;
                                         current_level++;   
                                         draw_menu ( 
                                                     menus[current_menu].start_x,
                                                     menus[current_menu].start_y,
                                                     menus[current_menu].start_x + menus[current_menu].width+2,
                                                     menus[current_menu].start_y + menus[current_menu].number_options+1,
                                                     current_menu,
                                                     FALSE
                                                    ); 
                                          current_menu = menu_table[current_menu][menu_option+1];
                                          open_menu (current_menu, current_level);
                                          menu_option = 0;
                                          highlight_option (current_menu, menu_option, TRUE);
                                      }
                                      else
                                      {
                                         close_menu (current_menu, current_level);
                                         highlight_menu_bar (current_menu, FALSE);
                                         current_menu++;
#ifdef DUMMY
                                         if ((current_menu == PROCESS_MENU) || (current_menu == SHARP_MENU))
                                                current_menu = EXIT_MENU;
#else
                                         if ((current_menu == PROCESS_MENU) &&
                                             !(ORGANISER_DEFINED)
                                            )
                                            current_menu = EXIT_MENU;
#endif                                 
                                         current_level = 0;
                                         if (current_menu > HELP_MENU)
                                            current_menu = FILE_MENU;
                                         menu_option = 0;
                                         highlight_menu_bar (current_menu, TRUE);
                                         open_menu (current_menu, current_level);
                                      }
                                   }
                                   else
                                   {
                                      if ((menus[current_menu].attrib[menu_option] == MENU_HAS_SUBS) &&
                                           (config_byte1 & MSK_MENU_TREE)
                                         )
                                      {
                                         level_option[current_level] = menu_option;
                                         current_level++;   
                                         draw_menu ( 
                                                     menus[current_menu].start_x,
                                                     menus[current_menu].start_y,
                                                     menus[current_menu].start_x + menus[current_menu].width+2,
                                                     menus[current_menu].start_y + menus[current_menu].number_options+1,
                                                     current_menu,
                                                     FALSE
                                                    ); 
                                         current_menu = menu_table[current_menu][menu_option+1];
                                         open_menu (current_menu, current_level);
                                         menu_option = 0;
                                         highlight_option (current_menu, menu_option, TRUE);
                                      }
                                      else
                                      {  
                                         shrink_menus ( current_level );
                                         highlight_menu_bar (last_menu, FALSE);
                                         current_menu = last_menu + 1;
                                         if (current_menu > HELP_MENU)
                                           current_menu = FILE_MENU;
                                         menu_option = 0;
                                         current_level = 0;
                                         open_menu (current_menu, current_level);
                                      }
                                   }
                                }   
                                break;
              case K_CSR_LT   : if (current_menu != -1)
                                {
                                   if (menu_table[current_menu][0] == -999)
                                   {
                                      close_menu (current_menu, current_level);
                                      highlight_menu_bar (current_menu, FALSE);
                                      current_menu--;
#ifdef DUMMY                                      
                                      if (current_menu == PROCESS_MENU)
                                        current_menu = MODEL_MENU;
#else
                                      if ((current_menu == PROCESS_MENU) && 
                                          !(ORGANISER_DEFINED)
                                         )
                                         {
                                           current_level = 0;
                                           current_menu = MODEL_MENU;
                                         }
#endif

                                      if (current_menu < FILE_MENU)
                                         current_menu = HELP_MENU;
                                      menu_option = 0;
                                      open_menu ( current_menu, current_level);
                                      highlight_option (current_menu, menu_option, TRUE);
                                      highlight_menu_bar (current_menu, TRUE);
                                   }
                                   else
                                   {
                                       close_menu (current_menu, current_level);
                                       current_level--;
                                       current_menu = menu_table[current_menu][0];
                                       menu_option = level_option[current_level];
                                       draw_menu ( 
                                                   menus[current_menu].start_x,
                                                   menus[current_menu].start_y,
                                                   menus[current_menu].start_x + menus[current_menu].width+2,
                                                   menus[current_menu].start_y + menus[current_menu].number_options+1,
                                                   current_menu,
                                                   TRUE
                                                  ); 
                                       highlight_option (current_menu, menu_option, TRUE);
                                   
                                   }
                                }
                                break;
              case K_F1       : show_help_main();
                                break;
              case K_ALT_E    : 
                                if (current_menu != -1)
                                {
                        
                                  highlight_menu_bar (current_menu, FALSE);
                                  close_menu (current_menu, current_level);
                                }
                                current_menu = EXIT_MENU;
                                current_level = 0;
                                highlight_menu_bar (current_menu, TRUE);
                                open_menu (current_menu, current_level);
                        
                                break;
              case K_ALT_H    : 
                                if (current_menu != -1)
                                {
                   
                                   highlight_menu_bar (current_menu, FALSE);
                                   close_menu (current_menu, current_level);
                                }
                                current_menu = HELP_MENU;
                                current_level = 0;
                                highlight_menu_bar (current_menu, TRUE);
                                open_menu (current_menu, current_level);
                  
                                break;
              case K_ALT_P    : 
#ifndef DUMMY
                                if (ORGANISER_DEFINED)
                                {
                                   if (current_menu != -1)
                                   {
                        
                                      highlight_menu_bar (current_menu, FALSE);
                                      close_menu (current_menu, current_level);
                                   }
                                   current_menu = PROCESS_MENU;
                                   current_level = 0;
                                   highlight_menu_bar (current_menu, current_level);
                                   open_menu (current_menu, current_level);
                                }
#endif
                                break;
              case K_ALT_C    : 
                                if (current_menu != -1)
                                {  
                               
                                   highlight_menu_bar (current_menu, FALSE);
                                   close_menu (current_menu, current_level);
                                }
                                current_menu = CONFIGURE_MENU;
                                current_level = 0;
                                highlight_menu_bar (current_menu, TRUE);
                                open_menu (current_menu, current_level);
              
                                break;
              case K_ALT_M    : 
                                if (current_menu != -1)
                                {
                                  
                                  highlight_menu_bar (current_menu, FALSE);
                                  close_menu (current_menu, current_level);
                                }
                          
                                current_menu = MODEL_MENU;
                                current_level = 0;
                                highlight_menu_bar (current_menu, TRUE);
                                open_menu (current_menu, current_level);
               
                                break;
              case K_ALT_F    : 
                                if (current_menu != -1)
                                {
                                  
                                  highlight_menu_bar (current_menu, FALSE);
                                  close_menu (current_menu, current_level);
                                }
                                current_menu = FILE_MENU;
                                current_level = 0;
                                highlight_menu_bar (current_menu, TRUE);
                                open_menu (current_menu, current_level);
                  
                                break;
                default         :
                                  break;
           }
         }
       }
       if (really_save_changes())
          save_config_file();

    }
    reset_video();
    ctrl_c (RELEASE);
}


