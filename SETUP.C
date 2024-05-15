#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include "keys.h"
#include "windows.h"
#include "config.h"
#include "iq.h"
#include "comm.h"
#include "palette.h"

extern int current_level;
extern struct palette_str palette;
extern unsigned char config_byte1;
extern unsigned char config_byte2;
extern struct comms_def comms;

extern void reset_video (void);
extern void init_video ( void );
extern void clear_area (int x1, int y1, int x2, int y2);
extern unsigned int get_key ( void );
extern void window_title ( int x1, int x2, int y, char msg[], short fg, short bk);
extern int get_yesno ( int x, int y, short fg, short bk);
extern void push_wait ( void );
extern void draw_button ( int x, int y, int width, char msg[], int state);
extern void simulate_push ( int x, int y, char msg[], int width);
extern void push_button ( int x, int y, char msg[]);
extern void centre_push_button (int x1, int x2, int y, char msg[]);
extern void WriteMultiChar (int x, int y, char ch, int n);
extern void shrink_menus ( int level );
extern void show_date ( void );
extern void show_time ( void );

unsigned char old_config_byte1;


void do_video_mode ( void );
void do_border_type ( void );
void do_shadow_type ( void );
void do_window_type ( void );
void do_update_method ( void );
void do_viewer_type ( void );
int abandon_option_changes ( void );
void date_options (int state);

int abandon_option_changes ()
{
   char *buff;
   int result;

   result = FALSE;
   buff = (char *)malloc(PORTSIZE(25, 9, 56, 13));
   if (buff != NULL)
   {
       save_area (25, 9, 56, 13, buff);   
       SetColourPair (palette.bright_white, palette.red);
       draw_window (25, 9, 55, 12);
       SetColourPair (palette.yellow, palette.red);
       WriteString (28, 10, "Abandon Setting Changes ?");
       result = get_yesno (34, 11, palette.yellow, palette.red);
       restore_area (25, 9, 56, 13, buff);
       free((char *)buff);
   }
   return(result);
}

/************************/
/* Video Mode Detection */
/************************/

int get_video_mode_option ( void );
void set_video_mode_option ( int option );
void mode_detect_help ( void );

int get_video_mode_option ()
{
    switch ( config_byte1 & MSK_VIDEO_MODE)    
    {
       case 0 : return(1); /* Mono */
       case 32 : return (2); /* Colour */
       case 64 : return (3); /* Auto */
       default : return(-1);
    }
}

void set_video_mode_option ( int option )
{
   config_byte1 &= ~MSK_VIDEO_MODE;
   switch ( option )
   {
      case 1 : break;
      case 2 : config_byte1 |= 32;
               break;
      case 3 : config_byte1 |= 64;
               break;
   }
}

void show_mode_setting ( int option )
{
   SetColourPair (palette.bright_white, palette.green);
   WriteChar (30, 12, ' ');
   WriteChar (30, 13, ' ');
   WriteChar (30, 14, ' ');
   WriteChar (30, 11+option, 'X');
}

void mode_detect_help ()
{
   char *hbuff;

   hbuff = (char *)malloc (PORTSIZE(20, 8, 61, 17));
   if (hbuff != NULL)
   {
     save_area (20, 8, 61, 17, hbuff);
     SetColourPair (palette.black, palette.green);
     draw_window (20, 8, 60, 16);
     window_title ( 20, 60, 8, "Video Mode Help", palette.red, palette.green);
     get_key();
     restore_area (20, 8, 61, 17, hbuff);
     free((char *)hbuff);
   }
}



void do_video_mode ()
{
   char *buff;
   unsigned int key;
   int option,
       finish;

   buff = (char *)malloc (PORTSIZE(25, 10, 56, 18));
   if (buff != NULL)
   {
      save_area (25,10, 56, 18, buff);
      SetColourPair (palette.black, palette.green);
      draw_window (25, 10, 55, 17);
      window_title ( 25, 55, 10, "Video Mode Detection", palette.red, palette.green);
      SetColourPair (palette.black, palette.green);
      WriteString ( 28, 12, "[   ] Monochrome");
      WriteString ( 28, 13, "[   ] Colour");
      WriteString ( 28, 14, "[   ] Auto Detect");
      SetColourPair (palette.red, palette.green);
      WriteChar (34, 12, 'M');
      WriteChar (36, 13, 'l');
      WriteChar (34, 14, 'A');
      draw_button ( 32, 16, 4, "&Ok", BUTTON_UP);
      draw_button ( 41, 16, 8, "&Cancel", BUTTON_UP);
      finish = FALSE;
      option = get_video_mode_option();
      old_config_byte1 = config_byte1;
      while (! finish)
      {
         show_mode_setting ( option );
         key = get_key();
         switch ( key )
         {
            case K_TAB    :
            case K_CSR_DN : option++;
                            if (option > 3)
                              option = 1;
                            break;
            case K_REVTAB :
            case K_CSR_UP : option--;
                            if ( option < 1)
                                option = 3;
                            break;

            case 'M'      : 
            case 'm'      : option = 1;
                            break;
            case 'L'      : 
            case 'l'      : option = 2;
                            break;
            case 'A'      : 
            case 'a'      : option = 3;
                            break;
            case K_F1     : mode_detect_help ();
                            break;
            case K_CR     :
            case 'O'      : 
            case 'o'      :
                            simulate_push (32, 16, "&Ok", 4);
                            finish = TRUE;
                            break;
            case 'C'      :
            case K_ESC    : 
            case 'c'      : simulate_push (41, 16, "&Cancel", 8);
                            if (config_byte1 != old_config_byte1)
                            {
                                if (abandon_option_changes())
                                   config_byte1 = old_config_byte1;
                            }
                            finish = TRUE;
                            break;
            default       : break;
         }

         if (! finish)
            set_video_mode_option ( option );
      }
      
      restore_area (25, 10, 56, 18, buff);
      free ((char *)buff);
      
/*      shrink_menus ( current_level ); */
      reset_video();
      init_video();
   }
}

/*************************/
/* Border Type Selection */
/*************************/

void do_border_type ( void );
void show_border_settings ( int option );
void do_border_help ( void );

void do_border_help ()
{
  char *hbuff;

  hbuff = (char *)malloc(PORTSIZE(20, 8, 61, 16));
  if (hbuff != NULL)
  {
     save_area (20, 8, 61, 16, hbuff);
     SetColourPair (palette.black, palette.green);
     draw_window (20, 8, 60, 15);
     window_title (20, 60, 8, "Border Type Help", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (23, 10, "The border type setting is used");
     WriteString (23, 11, "to determine which characters are");
     WriteString (23, 12, "used to draw window outlines.");
     push_button (35, 14,"&Continue");
     restore_area (20, 8, 61, 16, hbuff);
     free ((char *)hbuff);
  }
}

void show_border_setting ( int option )
{
  int i;

  SetColourPair (palette.bright_white, palette.green);
  WriteChar (25, 10, ' ');
  WriteChar (25, 11, ' ');
  WriteChar (25,9 + option,'X');
  SetColourPair (palette.bright_white, palette.blue);
  switch ( option )
  {
     case 1 : 
              WriteChar (45, 9, TL_SINGLE); WriteChar (56, 9, TR_SINGLE);
              WriteChar (45, 12, BL_SINGLE); WriteChar (56, 12, BR_SINGLE);
              WriteMultiChar (46, 9, H_SINGLE, 10);
              WriteMultiChar (46,12, H_SINGLE, 10);
              for (i=10; i<12; i++)
              {
                WriteChar (45, i, V_SINGLE);
                WriteChar (56, i, V_SINGLE);
              }
              break;
     case 2 : 
              WriteChar (45, 9, TL_DOUBLE); WriteChar (56, 9, TR_DOUBLE);
              WriteChar (45, 12, BL_DOUBLE); WriteChar (56, 12, BR_DOUBLE);
              WriteMultiChar (46, 9, H_DOUBLE, 10);
              WriteMultiChar (46,12, H_DOUBLE, 10);
              for (i=10; i<12; i++)
              {
                WriteChar (45, i, V_DOUBLE);
                WriteChar (56, i, V_DOUBLE);
              }

              break;
  }
  SetColourPair (palette.light_red, palette.blue);
  clear_area  (46,10,55,11);
  WriteString (48,10, "Sample"); 
  WriteString (48, 11, "Window");
}

void do_border_type ( void )
{
  char         *buff;
  unsigned int key;
  int          option;
  int          finish;
  unsigned char old_config_byte1;

  buff = (char *)malloc(PORTSIZE(20, 7, 61, 16));
  if (buff != NULL)
  {
     save_area (20,7,61,16,buff);
     SetColourPair (palette.black, palette.green);
     draw_window (20,7,60,15);
     window_title (20,60,7,"Window Border Type", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (23, 10, "[   ] Single Line");
     WriteString (23, 11, "[   ] Double Line");
     SetColourPair (palette.red, palette.green);
     WriteChar (29,10, 'S'); WriteChar (29,11,'D');
     draw_button (30, 14, 4, "&Ok", BUTTON_UP);
     draw_button (44, 14, 8, "&Cancel", BUTTON_UP);
     old_config_byte1 = config_byte1;
     if (config_byte1 & MSK_BORDER_TYPE)
        option = 1;
     else
        option = 2;
     finish = FALSE;
     while (! finish)
     {
        config_byte1 &= ~MSK_BORDER_TYPE;
        if (option == 1)
          config_byte1 |= MSK_BORDER_TYPE;

        show_border_setting ( option );
        key = get_key();
        switch ( key )
        {
           case K_F1      : do_border_help ();
                            break;
           case K_CR      :
           case 'O'       : 
           case 'o'       : finish = TRUE;
                            simulate_push (30,14, "&Ok", 4);
                            break;
           case 'C'       : 
           case 'c'       :
           case K_ESC     : 
                            finish = TRUE;
                            simulate_push ( 44, 14, "&Cancel", 8);
                            if (config_byte1 != old_config_byte1)
                            {
                               if (abandon_option_changes ())
                                  config_byte1 = old_config_byte1;
                            }
                            break;
           case K_TAB     : 
           case K_CSR_DN  : option++;
                            if (option > 2) option=1;
                            break;
           case K_REVTAB  :
           case K_CSR_UP  : option--;
                            if (option<1)
                                option = 2;
                            break;
           case 'S'       : 
           case 's'       : option=1;
                            break;
           case 'D'       : 
           case 'd'       : option=2;
                            break;
           default        : break;
        }
     }
     restore_area (20, 7, 61, 16, buff);
     free((char *)buff);
  }
}

/*************************/
/* Shadow Type Selection */
/*************************/

void do_shadow_type ( void );
void show_shadow_type ( void );
void do_shadow_help ( void );

void do_shadow_help ()
{
  char *hbuff;

  hbuff = (char *)malloc(PORTSIZE(20, 8, 61, 19));
  if (hbuff != NULL)
  {
    save_area (20, 8,61,19,hbuff);
    SetColourPair (palette.black, palette.green);
    draw_window(20,8,60,18);
    window_title(20,60,8,"Window Shadow Help", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString (23, 10, "The window shadow setting is used");
    WriteString (23, 11, "to determine whether windows are");
    WriteString (23, 12, "drawn as flat overlays (no shadow)");
    WriteString (23, 13, "or with a three-dimensional effect");
    WriteString (23, 14, "(with shadow).");
    centre_push_button (20, 60, 17, "&Continue");
    restore_area(20,8,61,19,hbuff);
    free((char *)hbuff);
  }
}

void show_shadow_type ()
{
  SetColourPair (palette.bright_white, palette.green);  
  if (config_byte1 & MSK_SHADOW_TYPE)
    WriteChar (25, 11, 'X');
  else
    WriteChar (25, 11, ' ');
  
  SetColourPair (palette.cyan, palette.green);
  clear_area (45, 9, 56, 13);
  SetColourPair (palette.cyan, palette.green);
  draw_window (46, 10, 55, 12);
  SetColourPair (palette.bright_white, palette.green);
  WriteString (48, 11, "Sample");
}

void do_shadow_type ()
{
  char          *buff;
  unsigned int  key;
  unsigned char old_config_byte1;
  int           finished;


  buff = (char *)malloc(PORTSIZE(20, 8, 61, 17));
  if (buff != NULL)
  {
     save_area (20, 8, 61, 17, buff);
     SetColourPair (palette.black, palette.green);
     draw_window (20, 8, 60, 16);
     window_title (20, 60, 8, "Window Shadow", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (23, 11, "[   ] Display Shadow");
     SetColourPair (palette.red, palette.green);
     WriteChar (29, 11, 'D');
     finished = FALSE;
     old_config_byte1 = config_byte1;
     draw_button (29, 15, 4, "&Ok", BUTTON_UP);
     draw_button (44, 15, 8, "&Cancel", BUTTON_UP);
     while (! finished)
     {
       show_shadow_type();
       key = get_key();
       switch ( key )
       {
          case 'O'   :
          case 'o'   :
          case K_CR  : simulate_push (29, 15, "&Ok", 4);
                       finished = TRUE;
                       break;
          case 'C'   :
          case 'c'   :
          case K_ESC : finished = TRUE;
                       simulate_push (44, 15, "&Cancel", 8);
                       if (config_byte1 != old_config_byte1)
                       {
                          if ( abandon_option_changes())
                             config_byte1 = old_config_byte1;
                       }
                       break;
          case 'D'     : 
          case 'd'     :
          case K_SPACE :if (config_byte1 & MSK_SHADOW_TYPE)
                           config_byte1 &= ~MSK_SHADOW_TYPE;
                         else
                            config_byte1 |= MSK_SHADOW_TYPE;
                         break;
          case K_F1  : do_shadow_help();
                       break;
          default    :
                       break;
       }
     }
     restore_area (20, 8, 61, 17, buff);
     free ((char *)buff);
  }
}

/***************/
/* Window Type */
/***************/

void do_window_type ( void );
void show_window_settings ( int option );

void show_window_settings ( int option)
{
   SetColourPair (palette.bright_white, palette.green);
   WriteChar (25, 10, ' '); WriteChar (25, 11, ' ');
   WriteChar (25, 9+option, 'X');
}

void do_window_type ()
{
  char          *buff;
  unsigned int  key;
  int           finish,
                option;
  unsigned char old_config_byte1;

  buff = (char *)malloc(PORTSIZE(20, 8, 61, 15));
  if (buff != NULL)
  {
    save_area (20, 8, 61, 15, buff);
    SetColourPair (palette.black, palette.green);
    draw_window (20, 8, 60, 14);
    window_title(20,60,8,"Window Type", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString (23, 10, "[   ]   Exploding Windows");
    WriteString (23, 11, "[   ]   Normal Windows");
    SetColourPair (palette.red, palette.green);
    WriteChar (31, 10, 'E'); WriteChar (31, 11, 'N');
    if (config_byte1 & MSK_WINDOW_TYPE)
      option = 1;
    else
     option = 2;
    finish = FALSE;
    draw_button(28, 13, 4, "&Ok", BUTTON_UP);
    draw_button(45, 13, 8, "&Cancel", BUTTON_UP);
    old_config_byte1 = config_byte1;
    while (! finish )
    {
       config_byte1 &= ~MSK_WINDOW_TYPE;
       if (option == 1)
          config_byte1 |= MSK_WINDOW_TYPE;

       show_window_settings (option);
       key = get_key();
       switch (key )
       {
          case 'E'      : 
          case 'e'      : option = 1;
                          break;
          case 'N'      : 
          case 'n'      : option = 2;
                          break;
          case K_TAB    :
          case K_CSR_DN : option++;
                          if (option>2)
                            option=1;
                          break;
          case K_REVTAB :
          case K_CSR_UP : option--;
                          if (option<1)
                            option=2;
                          break;
          case 'O'      :
          case 'o'      :
          case K_CR     : simulate_push (28, 13, "&Ok", 4);
                          finish = TRUE;
                          break;
          case K_ESC    :
          case 'C'      : 
          case 'c'      : simulate_push (45, 13, "&Cancel", 8);
                          finish = TRUE;
                          if (config_byte1 != old_config_byte1)
                          {
                             if (abandon_option_changes())
                                config_byte1 = old_config_byte1;
                          }
                          break;
          default       : break;
       }
    }
    restore_area (20,8,61,15,buff);
    free((char *)buff);
  }
}

/************************/
/* Screen Update Method */
/************************/

void do_update_method(void);
void show_update_options (int option);
void show_sample_update ( int colour, int option );
void clear_sample_update ( void );

void show_update_options ( int option )
{
   SetColourPair (palette.bright_white, palette.green);
   WriteChar (20, 10, ' ');
   WriteChar (20, 11, ' ');
   WriteChar (20, 9+option, 'X');
}

void show_sample_update (int colour, int option)
{
    config_byte1 &= ~MSK_MEMORY_WRITE;
    if (option == 2)
       config_byte1 |= MSK_MEMORY_WRITE;
    SetColourPair (palette.bright_white, colour);
    draw_window (48, 9, 60, 11);
    clear_area (49, 10, 59, 10);
}

void clear_sample_update ()
{
   SetColourPair (palette.green, palette.green);
   clear_area (48, 9, 60, 11);
}

void do_update_method ()
{
   char *buff;
   int finish,
       option;
   unsigned int key;
   unsigned char old_config_byte1;
   unsigned char copy_byte;
   int colour;

   buff = (char *)malloc(PORTSIZE(15, 8, 66, 16));
   if (buff != NULL)
   {
      save_area (15, 8, 66, 16, buff);
      SetColourPair (palette.black, palette.green);
      draw_window (15, 8, 65, 15);
      window_title (15, 65, 8, "Screen Updates", palette.red, palette.green);
      SetColourPair (palette.black, palette.green);
      WriteString (18, 10, "[   ]  BIOS Calls");
      WriteString (18, 11, "[   ]  Direct Memory Access");
      SetColourPair (palette.red, palette.green);
      WriteChar (25, 10, 'B'); WriteChar (25, 11, 'D');
      draw_button (26, 14, 4, "&Ok", BUTTON_UP);
      draw_button (44, 14, 8, "&Cancel", BUTTON_UP);
      finish = FALSE;
      old_config_byte1 = config_byte1;
      if (config_byte1 & MSK_MEMORY_WRITE)
         option = 2;
      else
         option=1;
      colour = 0;
      while (! finish)
      {
          config_byte1 &= ~MSK_MEMORY_WRITE;
          if (option==2)
             config_byte1 |= MSK_MEMORY_WRITE;
          show_update_options(option);
          
          copy_byte = config_byte1;
          while (!kbhit())   
          {
             show_sample_update(colour, option);
             colour++;
             if (colour>7)
                colour=0;
          }
          config_byte1 = copy_byte;

          key = get_key();
          switch ( key )
          {
              case 'O'      :
              case 'o'      :
              case K_CR     : finish = TRUE;
                              clear_sample_update;
                              simulate_push (26, 14, "&Ok",4);
                              break;
              case 'C'      :
              case 'c'      :
              case K_ESC    : finish = TRUE;
                              clear_sample_update;
                              simulate_push (44,14, "&Cancel", 8);
                              if (old_config_byte1 != config_byte1)
                              {
                                  if (abandon_option_changes())
                                     config_byte1 = old_config_byte1;
                              }
                              break;
              case K_CSR_UP :
              case K_REVTAB : option--;
                              if (option<1) option=2;
                              break;
              case K_CSR_DN :
              case K_TAB    : option++;
                              if (option > 2) option=1;
                              break;
              case 'B'      : 
              case 'b'      : option=1;
                              break;
              case 'D'      : 
              case 'd'      : option = 2;
                              break;
              default       : break;

          }
      }
      restore_area (15, 8, 66, 16, buff);
      free ((char *)buff);
   }
}

/***************/
/* Viewer Type */
/***************/

void do_viewer_type ( void );
void show_viewer_settings(int option);

void show_viewer_settings ( int option)
{
  SetColourPair (palette.bright_white, palette.green);
  WriteChar (29, 10, ' '); WriteChar(29,11, ' ');
  WriteChar (29, 9+option, 'X');
}

void do_viewer_type ()
{
   char         *buff;
   unsigned int key;
   int          finish,
                option;
   unsigned char old_config_byte1;
   
   buff=(char *)malloc(PORTSIZE(24, 8, 57, 15));
   if (buff != NULL)
   {
     save_area (24,8,57,15,buff);
     SetColourPair (palette.black, palette.green);
     draw_window(24,8,56,14);
     window_title (24,56,8,"File Viewer",palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (27, 10, "[   ]  ASCII Viewer");
     WriteString (27, 11, "[   ]  Hex Viewer");
     SetColourPair (palette.red, palette.green);
     WriteChar (34, 10, 'A'); WriteChar (34, 11, 'H');
     draw_button (32, 13, 4, "&Ok", BUTTON_UP);
     draw_button (42, 13, 8, "&Cancel", BUTTON_UP);
     finish = FALSE;
     if (config_byte1 & MSK_FILE_VIEWER)
       option=2;
     else
       option=1;
     old_config_byte1 = config_byte1;
     while (! finish)
     {
       config_byte1 &= ~MSK_FILE_VIEWER;
       if (option == 2)
         config_byte1 |= MSK_FILE_VIEWER;

       show_viewer_settings(option);
       key = get_key();
       switch (key)
       {             
          case K_CR     :
          case 'O'      : 
          case 'o'      : finish = TRUE;
                          simulate_push (32, 13, "&Ok", 4);
                          break;
          case K_CSR_UP :
          case K_REVTAB : option--;
                         if (option<1)
                           option=2;
                         break;
          case K_CSR_DN :
          case K_TAB    : option++;
                          if(option>2)
                            option=1;
                          break;
          case 'A'      : 
          case 'a'      : option=1;
                          break;
          case 'H'      : 
          case 'h'      : option=2;
                          break;
          case K_ESC    :
          case 'C'      : 
          case 'c'      : finish = TRUE;
                          simulate_push (42,13,"&Cancel", 8);
                          if (old_config_byte1 != config_byte1)
                          {
                              if (abandon_option_changes())
                                config_byte1 = old_config_byte1;
                          }
                          break;
          default      : break;
       }
     }
     restore_area (24, 8, 57, 15, buff);
     free((char *)buff);
   }
}

/*************************/
/* Configuration Summary */
/*************************/

void show_config_summary ( void );


void show_config_summary ()
{
   char *summbuff;
   char  str[80];

   summbuff = (char *)malloc(PORTSIZE(15, 3, 66, 22));
   if (summbuff != NULL)
   {
     save_area (15, 3, 66, 22, summbuff);
     SetColourPair (palette.black, palette.green);
     draw_window (15, 3, 65, 21);
     window_title (15, 65, 3, "Configuration Summary", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (19,  5, "Video Mode Selection");
     WriteString (19,  6, "Border Type");
     WriteString (19,  7, "Draw Shadow ?");
     WriteString (19,  8, "Window Type");
     WriteString (19,  9, "Screen Update Method");
     WriteString (19, 10, "File Viewer");

     WriteString (19, 12, "Serial Port (    ) Set To ");
     WriteString (19, 13, "Flow Control");
     WriteString (19, 15, "Time Format");
     WriteString (19, 16, "Date Format");
     WriteString (19, 17, "File Transfer");
     WriteString (19, 18, "Show Child Menu");

     SetColourPair (palette.bright_white, palette.green);
     switch (config_byte1 & MSK_VIDEO_MODE)
     {
        case 0  : WriteString (42, 5, "Monochrome");
                  break;
        case 32 : WriteString (42, 5, "Colour");
                  break;
        case 64 : WriteString (42, 5, "Auto Detect");
                  break;
     }
     if ( config_byte1 & MSK_BORDER_TYPE)
       WriteString (42, 6, "Single Line");
     else
       WriteString (42, 6, "Double Line");
     if (config_byte1 & MSK_SHADOW_TYPE)
       WriteString (42, 7, "Yes");
     else
       WriteString (42, 7, "No");
     if (config_byte1 & MSK_WINDOW_TYPE)
       WriteString (42, 8, "Exploding");
     else
       WriteString (42, 8, "Normal");
     if (config_byte1 & MSK_MEMORY_WRITE)
       WriteString (42, 9, "Direct To Memory");
     else
       WriteString (42, 9, "Change Via BIOS Calls");

     if (config_byte1 & MSK_FILE_VIEWER)
       WriteString (42, 10, "Hex Viewer");
     else
       WriteString (42, 10, "ASCII Viewer");
     WriteString (32, 12, "COM");
     WriteChar (35, 12, (char)(0x30+comms.port+1));
     switch ( comms.speed)
     {
        case BAUD_300 : strcpy (str, "300,");
                        break;
        case BAUD_1200: strcpy (str, "1200,");
                        break;
        case BAUD_2400: strcpy (str, "2400,");
                        break;
        case BAUD_4800: strcpy (str, "4800,");
                        break;
        case BAUD_9600: strcpy (str, "9600,");
                        break;
        default       : break;
     }
     switch (comms.parity)
     {
        case PARITY_NONE : strcat (str, "N,");
                           break;
        case PARITY_ODD  : strcat (str, "O,");
                           break;
        case PARITY_EVEN : strcat (str, "E,");
                           break;
        default          : break;
     }
     switch ( comms.data)
     {
        case DATA_BITS7 : strcat (str, "7,");
                          break;
        case DATA_BITS8 : strcat (str, "8,");
                          break;
        default         : break;
     }
     switch ( comms.stop)
     {
         case STOP_BITS1 : strcat (str, "1");
                           break;
         case STOP_BITS2 : strcat (str, "2");
                           break;
     }
     WriteString (45, 12, str);
     if (comms.flow)
       WriteString (45, 13, "XON/XOFF");
     else
       WriteString (45, 13, "Hardware");

     switch ((config_byte2 & MSK_DATE_FORMAT) >> 1)
     {
        case 0 : strcpy (str, "DD/MM/YY");
                 break;
        case 1 : strcpy (str, "MM/DD/YY");
                 break;
        case 2 : strcpy ( str, "YY/MM/DD");
                 break;
        case 3 : strcpy ( str, "YY/DD/MM");
                 break;
        case 4 : strcpy ( str, "DD/YY/MM");
                 break;
        case 5 : strcpy ( str, "MM/YY/DD");
                 break;
     }
     if (config_byte2 & MSK_DATE_ABBREVIATION)
        strcat ( str, " (ABBREVIATED)");
     else
       strcat ( str, " (FULL)");
     WriteString (42, 16, str);

     if (config_byte2 & MSK_TIME_FORMAT)
       strcpy (str, "HH:MM:SS");
     else
       strcpy (str, "HH:MM");
     if (config_byte2 & MSK_TIME_FRAME)
       strcat ( str, " (24 Hour)");
     else
       strcat ( str, " (12 Hour)");
     WriteString (42, 15, str);
     if (config_byte2 & MSK_TRANS_MODE)
         WriteString (42, 17, "Replace");
     else
         WriteString (42, 17, "Append ");
     if (config_byte1 & MSK_MENU_TREE)
       WriteString(42,18, "Yes");
     else
       WriteString(42,18, "No ");

     centre_push_button ( 15, 65, 20, "&Continue");
     restore_area (15, 3, 66, 22, summbuff);
     free((char *)summbuff);
   }
}

/***************/
/* Date Format */
/***************/

void show_date_format ( int option, int abbreviate, int show_day);
void do_date_format ( void );

void show_date_format ( int option, int abbreviate, int show_day )
{
   int i;

   SetColourPair (palette.bright_white, palette.green);
   for (i=11; i<=13;i++)
   {
     WriteChar (25, i, ' '); WriteChar (45, i, ' ');
   }
   WriteChar (25, 15, ' ');
   WriteChar (25, 16, ' ');
   if (abbreviate)
     WriteChar (25, 15, 'X');
   if (show_day && !abbreviate)
     WriteChar (25, 16, 'X');
   if (option < 3)
     WriteChar (25, 11+option, 'X');
   else
     WriteChar (45, 11+(option-3), 'X');
}

void date_options (int abbreviate)
{
    SetColourPair (palette.black, palette.green);
    if (abbreviate)
        SetColourPair (palette.white, palette.green);
    WriteString (23, 16, "[   ] Show Day Name");
  
    if (!abbreviate)
    {
       SetColourPair (palette.red, palette.green);
       WriteChar (29, 16, 'S');
    }
}

void do_date_format ()
{
  char          *buff;
  unsigned int  key;
  int           finish,
                abbreviate,
                show_day,
                option;
  unsigned char old_config_byte2;

  buff = (char *)malloc(PORTSIZE(20, 8, 61, 20));
  if (buff != NULL)
  {
    save_area (20, 8, 61, 20, buff);
    SetColourPair (palette.black, palette.green);
    draw_window (20, 8, 60, 19);
    window_title (20, 60, 8, "Date Format", palette.red, palette.green);
    SetColourPair (palette.black, palette.green);
    WriteString (23, 9, "Date Formats :");
    WriteString (23, 11, "[   ]  DD/MM/YY");
    WriteString (23, 12, "[   ]  MM/DD/YY");
    WriteString (23, 13, "[   ]  YY/MM/DD");
    WriteString (43, 11, "[   ]  YY/DD/MM");
    WriteString (43, 12, "[   ]  DD/YY/MM");
    WriteString (43, 13, "[   ]  MM/YY/DD");
    WriteString (23, 15, "[   ] Abbreviate Month Names");
    date_options(config_byte2 & MSK_DATE_ABBREVIATION);
    SetColourPair (palette.red, palette.green);
    WriteChar (29, 15, 'A');
    draw_button ( 28, 18, 4, "&Ok", BUTTON_UP);
    draw_button ( 44, 18, 8, "&Cancel", BUTTON_UP);
    finish = FALSE;
    option = ((config_byte2 & MSK_DATE_FORMAT) >> 1);
    abbreviate = config_byte2 & MSK_DATE_ABBREVIATION;
    show_day = config_byte2 & MSK_DAY_NAME;
    old_config_byte2 = config_byte2;
    while (! finish)
    {
       config_byte2 &= ~(MSK_DATE_FORMAT | MSK_DATE_ABBREVIATION | MSK_DAY_NAME);
       config_byte2 |= (option << 1); 
       config_byte2 |= abbreviate;
       config_byte2 |= show_day;
       
       show_date_format ( option , abbreviate, show_day);
       key = get_key();
       switch (key)
       {
          case K_REVTAB :
          case K_CSR_UP : option--;
                          if (option < 0)
                             option = 5;
                          break;
          case K_TAB    :
          case K_CSR_DN : option++;
                          if (option > 5)
                            option=0;
                          break;
          case K_ESC    :
          case 'C'      : 
          case 'c'      : finish = TRUE;
                          simulate_push (44, 18, "&Cancel", 8);
                          if (old_config_byte2 != config_byte2)
                          {
                             if (abandon_option_changes())
                                config_byte2 = old_config_byte2;
                             else
                             {
                                  config_byte2 &= ~(MSK_DATE_FORMAT | MSK_DATE_ABBREVIATION | MSK_DAY_NAME);
                                  config_byte2 |= (option << 1); 
                                  if (abbreviate)
                                        config_byte2 |= MSK_DATE_ABBREVIATION;
                                  if (show_day)
                                        config_byte2 |= MSK_DAY_NAME;
                             }
                          }
                          break;
          case K_CR     :
          case 'O'      : 
          case 'o'      : finish = TRUE;
                          config_byte2 &= ~(MSK_DATE_FORMAT | MSK_DATE_ABBREVIATION | MSK_DAY_NAME);
                          config_byte2 |= (option << 1); 
                          if (abbreviate)
                                config_byte2 |= MSK_DATE_ABBREVIATION;
                          if (show_day)
                                config_byte2 |= MSK_DAY_NAME;
                          
                          simulate_push (28, 18, "&Ok", 4);
                          break;
          case 'A'      : 
          case 'a'      : abbreviate = !abbreviate;
                          date_options(abbreviate);
                          break;
          case 'S'      : 
          case 's'      : if (!abbreviate)
                                show_day = !show_day;
                          break;
          default       : break;
       }
    }
    restore_area (20, 8, 61, 20, buff);
    free ((char *)buff);
  }
  show_date();
}

/***************/
/* Time Format */
/***************/

void do_time_format ( void );
void show_time_options (int option);

void show_time_options( int option )
{
   SetColourPair (palette.bright_white, palette.green);
   WriteChar (30, 10, ' ');
   WriteChar (30, 11, ' ');
   WriteChar (30, 13, ' ');
   if (config_byte2 & MSK_TIME_FRAME)
     WriteChar (30, 13, 'X');
   WriteChar (30, 9+option, 'X');
}

void do_time_format ()
{
   char          *buff;
   int           finish,
                 frame,
                 option;
   unsigned char old_config_byte2;
   unsigned int key;

   buff = (char *)malloc(PORTSIZE(25, 8, 56, 17));
   if (buff != NULL)
   {
     save_area (25, 8, 56, 17, buff);
     SetColourPair (palette.black, palette.green);
     draw_window (25, 8, 55, 16);
     window_title (25, 55, 8, "Time Format", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (28, 10, "[   ]   HH:MM:SS");
     WriteString (28, 11, "[   ]   HH:MM");
     WriteString (28, 13, "[   ]   24-Hour Format");
     SetColourPair (palette.red, palette.green);
     WriteChar (44, 13, 'F');
     draw_button (32, 15, 4, "&Ok", BUTTON_UP);
     draw_button ( 42, 15, 8, "&Cancel", BUTTON_UP);
     finish = FALSE;
     if (config_byte2 & MSK_TIME_FORMAT)
        option = 1;
     else
       option = 2;
     frame = config_byte2 & MSK_TIME_FRAME;
     old_config_byte2 = config_byte2;
     while (! finish)
     {
        config_byte2 &= ~(MSK_TIME_FORMAT | MSK_TIME_FRAME);
        if (frame)
           config_byte2 |= MSK_TIME_FRAME;
        if (option == 1)
          config_byte2 |= MSK_TIME_FORMAT;

        show_time_options(option);
        key = get_key();
        switch (key )
        {
           case K_CR     :
           case 'O'      : 
           case 'o'      : finish = TRUE;
                           simulate_push (32, 15, "&Ok", 4);
                           break;
           case K_CSR_UP :
           case K_REVTAB : option--;
                           if (option<1)
                              option=2;
                           break;
           case K_TAB    :
           case K_CSR_DN : option++;
                           if (option>2)
                              option=1;
                           break;
           case K_ESC    :
           case 'C'      : 
           case 'c'      : finish = TRUE;
                           simulate_push (42, 15, "&Cancel", 8);
                           if (old_config_byte2 != config_byte2)
                           {
                              if (abandon_option_changes())
                                config_byte2 = old_config_byte2;
                           }
                           break;
           case 'F'      : 
           case 'f'      : frame = !frame;
                           break;
           default       : break;

        }
     }
     restore_area (25, 8, 56, 17, buff);
     free ((char *)buff);
   }
   show_time();
}
