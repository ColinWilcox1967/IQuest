#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "palette.h"
#include "iq.h"
#include "windows.h"
#include "config.h"
#include "menu.h"

extern unsigned int get_key ( void );
extern void save_area (int x1, int x2, int y1, int y2, char *buf);
extern void restore_area (int x1, int y1, int x2, int y2, char *buf);
extern void draw_window (int x1, int y1, int x2, int y2);
extern void SetColourPair (short fg, short bk);
extern void WriteString (int x, int y, char s[]);
extern void WriteMultiChar (int x, int y, char ch, int n);
extern void show_menu_bar ( void );

extern int file_list_count;
extern struct palette_str palette;
extern unsigned char config_byte1;
extern char current_organiser[];

int current_level;
int current_menu;
int last_menu;
int menu_option;

char   *menu_buff[MAX_MENU_LEVELS];
int     level_option[MAX_MENU_LEVELS];
struct  menu_def menus[MAX_MENUS];
int     menu_table[MAX_MENUS][1+MAX_MENU_OPTIONS];

void init_menus ( void );
void highlight_menu_bar (int option, int state);
void open_menu ( int m, int layer);
void shrink_menus ( int l );
void close_menu (int m, int layer);
void draw_menu (int x1, int y1, int x2, int y2, int mn, int state );

void highlight_menu_bar ( int option, int state)
{
   if (state)
     SetColourPair (palette.bright_white, palette.black);
   else
     SetColourPair (palette.black, palette.white);
   switch ( option )
   {
     case FILE_MENU      : WriteString (1,1, "File");
                           break; 
     case CONFIGURE_MENU : WriteString (9,1,"Configure");
                           break;
     case MODEL_MENU     : WriteString (23,1,"Model");
                           break;
     case PROCESS_MENU   : 
#ifdef DUMMY
                           SetColourPair (palette.black, palette.white);
                           WriteString (33, 1, "Process");
#else
                           if (strcmpi (current_organiser, "NONE") == 0)
                           {
                              SetColourPair (palette.black, palette.white);
                              WriteString (33, 1, "Process");
                           }
#endif
                           break;
     case EXIT_MENU      : WriteString (43,1,"Exit");
                           break;
     case HELP_MENU      : WriteString (67,1,"Help");
   }
   if (! state)
   {
     SetColourPair (palette.red, palette.white);
     switch (option)
     {
        case FILE_MENU      : WriteChar (1,1, 'F');
                              break;
        case CONFIGURE_MENU : WriteChar (9,1, 'C');
                              break;
        case MODEL_MENU     : WriteChar (23,1, 'M');
                              break;
        case PROCESS_MENU   : 

#ifdef DUMMY
                              SetColourPair (palette.black, palette.white);
#endif
                              if (strcmpi (current_organiser, "NONE") != 0)
                                WriteChar (33, 1, 'P');
                              break;
        case EXIT_MENU      : WriteChar (43,1, 'E');
                              break;
        case HELP_MENU      : WriteChar (67,1, 'H');
     }
   }
}

void init_menus ()
{
   int menu_number,
       menu_option;

   for (menu_number = 0; menu_number < MAX_MENU_LEVELS; menu_number++)
      menu_buff[menu_number] = NULL;

   for (menu_number = 0; menu_number < MAX_MENUS; menu_number++)
     for (menu_option = 0; menu_option <= MAX_MENU_OPTIONS; menu_option++)       
       menu_table[menu_number][menu_option] = -999;

   /* File */

   menu_table[FILE_MENU][1] = PRINTER;
   menu_table[FILE_MENU][2] = VIEW_FILE;
   menu_table[FILE_MENU][3] = DOS_COMMANDS;
   menu_table[FILE_MENU][5] = PREVIOUS_FILES;

   menu_table[PRINTER][0] = FILE_MENU;

   menu_table[PRINTER][1] = DO_REVIEW_PRINT;
   menu_table[PRINTER][3] = DO_PRINT;

   menu_table[DOS_COMMANDS][0] = FILE_MENU;
   menu_table[DOS_COMMANDS][1] = DOS_COPY;
   menu_table[DOS_COMMANDS][2] = DOS_REN;
   menu_table[DOS_COMMANDS][3] = DOS_DEL;
   menu_table[DOS_COMMANDS][5] = DOS_RD;
   menu_table[DOS_COMMANDS][6] = DOS_MD;

   /* Configure */

   menu_table[VIDCONF][0] = CONFIGURE_MENU; 
   menu_table[VIDCONF][1] = VIDEO_MODE;
   menu_table[VIDCONF][2] = BORDER_TYPE;
   menu_table[VIDCONF][3] = SHADOW_TYPE;
   menu_table[VIDCONF][4] = WINDOW_TYPE;
   menu_table[VIDCONF][5] = SCREEN_UPDATE_METHOD;
   menu_table[VIDCONF][6] = FILE_VIEWER;

   menu_table[CONFIGURE_MENU][1] = VIDCONF;
   menu_table[CONFIGURE_MENU][4] = APPEARANCE;
   menu_table[CONFIGURE_MENU][2] = PRINT_CONFIG;
   menu_table[CONFIGURE_MENU][3] = SELECT_PORT;
   menu_table[CONFIGURE_MENU][6] = CONFIG_SUMMARY;

   /* Config Printer */

   menu_table[PRINT_CONFIG][0] = CONFIGURE_MENU;
   menu_table[PRINT_CONFIG][1] = PRINTER_LOAD;
   menu_table[PRINT_CONFIG][2] = PRINTER_SAVE;
   menu_table[PRINT_CONFIG][3] = PRINTER_EDIT;

   /* Comms Config */

   menu_table[SELECT_PORT][0] = CONFIGURE_MENU;
   menu_table[SELECT_PORT][1] = SERIAL_PORT;
   menu_table[SELECT_PORT][2] = TRANS_MODE;

   menu_table[SERIAL_PORT][0] = SELECT_PORT;
   menu_table[SERIAL_PORT][1] = SELECT_COM1;
   menu_table[SERIAL_PORT][2] = SELECT_COM2;
   menu_table[SERIAL_PORT][3] = SELECT_COM3;
   menu_table[SERIAL_PORT][4] = SELECT_COM4;


   /* Model - Sharp */

   menu_table[MODEL_MENU][1] = SHARP_MENU;
   menu_table[SHARP_MENU][0] = MODEL_MENU;

   menu_table[SHARP_MENU][1] = SHARP_EL_MENU;
   menu_table[SHARP_EL_MENU][0] = SHARP_MENU;

   menu_table[SHARP_MENU][2] = SHARP_IQ_MENU;
   menu_table[SHARP_IQ_MENU][0] = SHARP_MENU;
   
   menu_table[SHARP_MENU][3] = SHARP_ZQ_MENU;
   menu_table[SHARP_ZQ_MENU][0] = SHARP_MENU;

   menu_table[SHARP_MENU][4] = SHARP_XE_MENU;
   menu_table[SHARP_XE_MENU][0] = SHARP_MENU;

   menu_table[SHARP_XE_MENU][1] = XE_7000;

   menu_table[SHARP_EL_MENU][1] = EL6XXX_MENU;

   menu_table[SHARP_IQ_MENU][1] = IQ7XXX_MENU;
   menu_table[SHARP_IQ_MENU][2] = IQ8XXX_MENU;
   menu_table[SHARP_IQ_MENU][3] = IQ9XXX_MENU;

   menu_table[SHARP_ZQ_MENU][1] = ZQ1XXX_MENU;
   menu_table[SHARP_ZQ_MENU][2] = ZQ2XXX_MENU;
   menu_table[SHARP_ZQ_MENU][3] = ZQ3XXX_MENU;
   menu_table[SHARP_ZQ_MENU][4] = ZQ4XXX_MENU;
   menu_table[SHARP_ZQ_MENU][5] = ZQ5XXX_MENU;
   menu_table[SHARP_ZQ_MENU][6] = ZQ6XXX_MENU;

   menu_table[EL6XXX_MENU][0] = SHARP_EL_MENU;
   menu_table[IQ7XXX_MENU][0] = SHARP_IQ_MENU;
   menu_table[IQ8XXX_MENU][0] = SHARP_IQ_MENU;
   menu_table[IQ9XXX_MENU][0] = SHARP_IQ_MENU;
   menu_table[ZQ1XXX_MENU][0] = SHARP_ZQ_MENU;
   menu_table[ZQ2XXX_MENU][0] = SHARP_ZQ_MENU;
   menu_table[ZQ3XXX_MENU][0] = SHARP_ZQ_MENU;
   menu_table[ZQ4XXX_MENU][0] = SHARP_ZQ_MENU;
   menu_table[ZQ5XXX_MENU][0] = SHARP_ZQ_MENU;
   menu_table[ZQ6XXX_MENU][0] = SHARP_ZQ_MENU;


   menu_table[EL6XXX_MENU][1] = EL_6000;
   menu_table[EL6XXX_MENU][2] = EL_6170;
   menu_table[EL6XXX_MENU][3] = EL_6190;
   menu_table[EL6XXX_MENU][4] = EL_6320;
   menu_table[EL6XXX_MENU][5] = EL_6330;
   menu_table[EL6XXX_MENU][6] = EL_6360;
   menu_table[EL6XXX_MENU][7] = EL_6390;

   menu_table[IQ7XXX_MENU][1] = IQ_7000;
   menu_table[IQ7XXX_MENU][2] = IQ_7100M;
   menu_table[IQ7XXX_MENU][3] = IQ_7200;
   menu_table[IQ7XXX_MENU][4] = IQ_7300M;
   menu_table[IQ7XXX_MENU][5] = IQ_7400;
   menu_table[IQ7XXX_MENU][6] = IQ_7420;
   menu_table[IQ7XXX_MENU][7] = IQ_7600;
   menu_table[IQ7XXX_MENU][8] = IQ_7620;
   menu_table[IQ7XXX_MENU][9] = IQ_7690;

   menu_table[IQ8XXX_MENU][1] = IQ_8000;
   menu_table[IQ8XXX_MENU][2] = IQ_8100M;
   menu_table[IQ8XXX_MENU][3] = IQ_8200;
   menu_table[IQ8XXX_MENU][4] = IQ_8300M;
   menu_table[IQ8XXX_MENU][5] = IQ_8400;
   menu_table[IQ8XXX_MENU][6] = IQ_8500M;
   menu_table[IQ8XXX_MENU][7] = IQ_8600M;
   menu_table[IQ8XXX_MENU][8] = IQ_8900;
   menu_table[IQ8XXX_MENU][9] = IQ_8920;
   
   menu_table[IQ9XXX_MENU][1] = IQ_9000;
   menu_table[IQ9XXX_MENU][2] = IQ_9000MK2;
   menu_table[IQ9XXX_MENU][3] = IQ_9200;
   menu_table[IQ9XXX_MENU][4] = IQ_9200MK2;

   menu_table[ZQ1XXX_MENU][1] = ZQ_1000;
   menu_table[ZQ1XXX_MENU][2] = ZQ_1050;
   menu_table[ZQ1XXX_MENU][3] = ZQ_1200;
   menu_table[ZQ1XXX_MENU][4] = ZQ_1250A;

   menu_table[ZQ2XXX_MENU][1] = ZQ_2200;
   menu_table[ZQ2XXX_MENU][2] = ZQ_2250;
   menu_table[ZQ2XXX_MENU][3] = ZQ_2400;
   menu_table[ZQ2XXX_MENU][4] = ZQ_2450;
   menu_table[ZQ2XXX_MENU][5] = ZQ_2500;
   menu_table[ZQ2XXX_MENU][6] = ZQ_2700;
   menu_table[ZQ2XXX_MENU][7] = ZQ_2750;

   menu_table[ZQ3XXX_MENU][1] = ZQ_3000;
   menu_table[ZQ3XXX_MENU][2] = ZQ_3200;
   menu_table[ZQ3XXX_MENU][3] = ZQ_3250;

   menu_table[ZQ4XXX_MENU][1] = ZQ_4450;

   menu_table[ZQ5XXX_MENU][1] = ZQ_5000;
   menu_table[ZQ5XXX_MENU][2] = ZQ_5100M;
   menu_table[ZQ5XXX_MENU][3] = ZQ_5200;
   menu_table[ZQ5XXX_MENU][4] = ZQ_5300;
   menu_table[ZQ5XXX_MENU][5] = ZQ_5300M;

   menu_table[ZQ6XXX_MENU][1] = ZQ_6000;
   menu_table[ZQ6XXX_MENU][2] = ZQ_6100M;
   menu_table[ZQ6XXX_MENU][3] = ZQ_6200;
   menu_table[ZQ6XXX_MENU][4] = ZQ_6300M;

   menu_table[HELP_MENU][1] = DO_GLOSSARY;

   current_menu = -1; /* No Active Menu */
   current_level = -1; /* No level active */

   /* Process Menu */

#ifndef DUMMY
   menu_table[PROCESS_MENU][1] = GET_DATA;
   menu_table[PROCESS_MENU][2] = GET_PASSWORD;
#endif

   /* File Menu */

   menus[FILE_MENU].start_x = 1;  menus[FILE_MENU].start_y = 2;
   menus[FILE_MENU].width   = 15; menus[FILE_MENU].number_options = 5;
   menus[FILE_MENU].attrib[0] = MENU_HAS_SUBS; strcpy(menus[FILE_MENU].option[0], "Print"); 
   menus[FILE_MENU].attrib[1] = MENU_NORMAL; strcpy(menus[FILE_MENU].option[1], "View");
   menus[FILE_MENU].attrib[2] = MENU_HAS_SUBS; strcpy (menus[FILE_MENU].option[2], "DOS Commands");
   menus[FILE_MENU].attrib[3] = MENU_SEPARATOR; strcpy (menus[FILE_MENU].option[3], "");
   menus[FILE_MENU].attrib[4] = MENU_NORMAL; strcpy (menus[FILE_MENU].option[4], "Previous Files");
   
   menus[DOS_COMMANDS].start_x = menus[FILE_MENU].start_x+menus[FILE_MENU].width+3;  
   menus[DOS_COMMANDS].start_y = menus[FILE_MENU].start_y+3;
   menus[DOS_COMMANDS].width   = 17; menus[DOS_COMMANDS].number_options = 6;
   menus[DOS_COMMANDS].attrib[0] = MENU_NORMAL; strcpy(menus[DOS_COMMANDS].option[0], "Copy File"); 
   menus[DOS_COMMANDS].attrib[1] = MENU_NORMAL; strcpy(menus[DOS_COMMANDS].option[1], "Rename File"); 
   menus[DOS_COMMANDS].attrib[2] = MENU_NORMAL; strcpy(menus[DOS_COMMANDS].option[2], "Delete File"); 
   menus[DOS_COMMANDS].attrib[3] = MENU_SEPARATOR; strcpy (menus[DOS_COMMANDS].option[3], "");
   menus[DOS_COMMANDS].attrib[4] = MENU_NORMAL; strcpy(menus[DOS_COMMANDS].option[4], "Remove Directory"); 
   menus[DOS_COMMANDS].attrib[5] = MENU_NORMAL; strcpy(menus[DOS_COMMANDS].option[5], "Create Directory"); 

   /* Process Menu */

   menus[PROCESS_MENU].start_x = 33; menus[PROCESS_MENU].start_y = 2;

   menus[PROCESS_MENU].width = 22; menus[PROCESS_MENU].number_options = 2;

#ifdef DUMMY
   menus[PROCESS_MENU].attrib[0] = MENU_GREYED; strcpy (menus[PROCESS_MENU].option[0], "Transfer Data");
   menus[PROCESS_MENU].attrib[1] = MENU_GREYED; strcpy (menus[PROCESS_MENU].option[1], "Interrogate Organiser");
#else   
   menus[PROCESS_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[PROCESS_MENU].option[0], "Transfer Data");
   menus[PROCESS_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[PROCESS_MENU].option[1], "Interrogate Organiser");
#endif   

   /* Print Menu */

   menus[PRINTER].start_x = menus[FILE_MENU].start_x+menus[FILE_MENU].width+3;
   menus[PRINTER].start_y = menus[FILE_MENU].start_y+1;
   menus[PRINTER].width = 16; menus[PRINTER].number_options = 3;
   menus[PRINTER].attrib[0] = MENU_NORMAL; strcpy (menus[PRINTER].option[0], "Review Settings");
   menus[PRINTER].attrib[1] = MENU_SEPARATOR;
   menus[PRINTER].attrib[2] = MENU_NORMAL; strcpy (menus[PRINTER].option[2], "Print File");


   /* Configuration Menu */

   menus[CONFIGURE_MENU].start_x = 9; menus[CONFIGURE_MENU].start_y = 2;
   menus[CONFIGURE_MENU].width = 20; menus[CONFIGURE_MENU].number_options = 6;
   menus[CONFIGURE_MENU].attrib[0] = MENU_HAS_SUBS; strcpy(menus[CONFIGURE_MENU].option[0], "Video");
   menus[CONFIGURE_MENU].attrib[1] = MENU_HAS_SUBS; strcpy(menus[CONFIGURE_MENU].option[1], "Printer");
   menus[CONFIGURE_MENU].attrib[2] = MENU_HAS_SUBS; strcpy(menus[CONFIGURE_MENU].option[2], "Communications");
   menus[CONFIGURE_MENU].attrib[3] = MENU_HAS_SUBS; strcpy (menus[CONFIGURE_MENU].option[3], "Appearance");
   menus[CONFIGURE_MENU].attrib[4] = MENU_SEPARATOR; strcpy (menus[CONFIGURE_MENU].option[4], "");
   menus[CONFIGURE_MENU].attrib[5] = MENU_NORMAL; strcpy ( menus[CONFIGURE_MENU].option[5], "Summary");

   /* Configuration - Printer */

   menus[PRINT_CONFIG].start_x = menus[CONFIGURE_MENU].start_x+menus[CONFIGURE_MENU].width+3;
   menus[PRINT_CONFIG].start_y = menus[CONFIGURE_MENU].start_y+1+1;
   menus[PRINT_CONFIG].width=15; menus[PRINT_CONFIG].number_options = 3;
   menus[PRINT_CONFIG].attrib[0] = MENU_NORMAL; strcpy (menus[PRINT_CONFIG].option[0], "Load Printer");
   menus[PRINT_CONFIG].attrib[1] = MENU_NORMAL; strcpy (menus[PRINT_CONFIG].option[1], "Save Printer");
   menus[PRINT_CONFIG].attrib[2] = MENU_NORMAL; strcpy (menus[PRINT_CONFIG].option[2], "Edit Settings");


   /* Configuration - Comms */

   menus[SELECT_PORT].start_x = menus[CONFIGURE_MENU].start_x+menus[CONFIGURE_MENU].width+3;
   menus[SELECT_PORT].start_y = menus[CONFIGURE_MENU].start_y+1+2;
   menus[SELECT_PORT].width = 14; menus[SELECT_PORT].number_options=2;
   menus[SELECT_PORT].attrib[0] = MENU_HAS_SUBS; strcpy (menus[SELECT_PORT].option[0], "Port");
   menus[SELECT_PORT].attrib[1] = MENU_NORMAL; strcpy (menus[SELECT_PORT].option[1], "Transfer Mode");

   
   menus[SERIAL_PORT].start_x = menus[SELECT_PORT].start_x+menus[SELECT_PORT].width+3;
   menus[SERIAL_PORT].start_y = menus[SELECT_PORT].start_y+1;
   menus[SERIAL_PORT].width = 9; menus[SERIAL_PORT].number_options=4;
   menus[SERIAL_PORT].attrib[0] = MENU_NORMAL; strcpy (menus[SERIAL_PORT].option[0], "COM1");
   menus[SERIAL_PORT].attrib[1] = MENU_NORMAL; strcpy (menus[SERIAL_PORT].option[1], "COM2");
   menus[SERIAL_PORT].attrib[2] = MENU_NORMAL; strcpy (menus[SERIAL_PORT].option[2], "COM3");
   menus[SERIAL_PORT].attrib[3] = MENU_NORMAL; strcpy (menus[SERIAL_PORT].option[3], "COM4");
   
   
   menus[VIDCONF].start_x = menus[CONFIGURE_MENU].start_x + menus[CONFIGURE_MENU].width+3;
   menus[VIDCONF].start_y = menus[CONFIGURE_MENU].start_y+1;
   menus[VIDCONF].width = 16; menus[VIDCONF].number_options = 6;
   menus[VIDCONF].attrib[0] = MENU_NORMAL; strcpy (menus[VIDCONF].option[0], "Video Mode");
   menus[VIDCONF].attrib[1] = MENU_NORMAL; strcpy (menus[VIDCONF].option[1], "Borders Details");
   menus[VIDCONF].attrib[2] = MENU_NORMAL; strcpy (menus[VIDCONF].option[2], "Drop Shadow");
   menus[VIDCONF].attrib[3] = MENU_NORMAL; strcpy (menus[VIDCONF].option[3], "Windows Type");
   menus[VIDCONF].attrib[4] = MENU_NORMAL; strcpy (menus[VIDCONF].option[4], "Screen Updates");
   menus[VIDCONF].attrib[5] = MENU_NORMAL; strcpy (menus[VIDCONF].option[5], "File Viewer");

   /* Appearance Configure */

   menu_table[APPEARANCE][0] = CONFIGURE_MENU;
   menus[APPEARANCE].start_x = menus[CONFIGURE_MENU].start_x+menus[CONFIGURE_MENU].width+3;
   menus[APPEARANCE].start_y = menus[CONFIGURE_MENU].start_y+1+3;
   menus[APPEARANCE].width = 13; menus[APPEARANCE].number_options = 3;
   menus[APPEARANCE].attrib[0] = MENU_NORMAL; strcpy (menus[APPEARANCE].option[0], "System Time");
   menus[APPEARANCE].attrib[1] = MENU_NORMAL; strcpy (menus[APPEARANCE].option[1], "System Date");
   menus[APPEARANCE].attrib[2] = MENU_NORMAL; strcpy (menus[APPEARANCE].option[2], "Expand Menus");

   menu_table[APPEARANCE][1] = TIME_FORMAT;
   menu_table[APPEARANCE][2] = DATE_FORMAT;
   menu_table[APPEARANCE][3] = MENU_TREE;

   /* Model Menu */

   menus[MODEL_MENU].start_x = 23; menus[MODEL_MENU].start_y = 2;
   menus[MODEL_MENU].width = 8; menus[MODEL_MENU].number_options = 1;
   menus[MODEL_MENU].attrib[0] = MENU_HAS_SUBS; strcpy (menus[MODEL_MENU].option[0], "Sharp");


   /* Model Menu - Sharp */
   
   menus[SHARP_MENU].start_x = menus[MODEL_MENU].start_x + menus[MODEL_MENU].width + 3;
   menus[SHARP_MENU].start_y = menus[MODEL_MENU].start_y + 1;
   menus[SHARP_MENU].width = 11; menus[SHARP_MENU].number_options = 4;
   menus[SHARP_MENU].attrib[0] = MENU_HAS_SUBS; strcpy ( menus[SHARP_MENU].option[0], "EL-Series");
   menus[SHARP_MENU].attrib[1] = MENU_HAS_SUBS; strcpy ( menus[SHARP_MENU].option[1], "IQ-Series");
   menus[SHARP_MENU].attrib[2] = MENU_HAS_SUBS; strcpy ( menus[SHARP_MENU].option[2], "ZQ-Series");
   menus[SHARP_MENU].attrib[3] = MENU_HAS_SUBS; strcpy ( menus[SHARP_MENU].option[3], "XE-Series");


   menus[SHARP_XE_MENU].start_x = menus[SHARP_MENU].start_x + menus[SHARP_MENU].width + 3;
   menus[SHARP_XE_MENU].start_y = menus[SHARP_MENU].start_y+4;
   menus[SHARP_XE_MENU].width = 16; menus[SHARP_XE_MENU].number_options = 1;
   menus[SHARP_XE_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[SHARP_XE_MENU].option[0], "Xeraus XE-7000");

   /* Model Menu - EL Models */

   menus[SHARP_EL_MENU].start_x = menus[SHARP_MENU].start_x + menus[SHARP_MENU].width + 3;
   menus[SHARP_EL_MENU].start_y = menus[SHARP_MENU].start_y+1;
   menus[SHARP_EL_MENU].width = 10; menus[SHARP_EL_MENU].number_options = 1;
   menus[SHARP_EL_MENU].attrib[0] = MENU_HAS_SUBS; strcpy (menus[SHARP_EL_MENU].option[0], "EL-6XXX");

   /* Model Menu - IQ Models */

   menus[SHARP_IQ_MENU].start_x = menus[SHARP_MENU].start_x + menus[SHARP_MENU].width + 3;
   menus[SHARP_IQ_MENU].start_y = menus[SHARP_MENU].start_y + 1+1;
   menus[SHARP_IQ_MENU].width = 11; menus[SHARP_IQ_MENU].number_options = 3;
   menus[SHARP_IQ_MENU].attrib[0] = MENU_HAS_SUBS; strcpy (menus[SHARP_IQ_MENU].option[0], "IQ-7XXX");
   menus[SHARP_IQ_MENU].attrib[1]= MENU_HAS_SUBS; strcpy (menus[SHARP_IQ_MENU].option[1], "IQ-8XXX");   
   menus[SHARP_IQ_MENU].attrib[2] = MENU_HAS_SUBS; strcpy (menus[SHARP_IQ_MENU].option[2], "IQ-9XXX");   
   
   /* Model Menu - ZQ Models */

   menus[SHARP_ZQ_MENU].start_x = menus[SHARP_MENU].start_x + menus[SHARP_MENU].width + 3;
   menus[SHARP_ZQ_MENU].start_y = menus[SHARP_MENU].start_y + 1+2;
   menus[SHARP_ZQ_MENU].width = 11; menus[SHARP_ZQ_MENU].number_options = 6;
   menus[SHARP_ZQ_MENU].attrib[0] = MENU_HAS_SUBS; strcpy (menus[SHARP_ZQ_MENU].option[0], "ZQ-1XXX");
   menus[SHARP_ZQ_MENU].attrib[1] = MENU_HAS_SUBS; strcpy (menus[SHARP_ZQ_MENU].option[1], "ZQ-2XXX");
   menus[SHARP_ZQ_MENU].attrib[2] = MENU_HAS_SUBS; strcpy (menus[SHARP_ZQ_MENU].option[2], "ZQ-3XXX");  
   menus[SHARP_ZQ_MENU].attrib[3] = MENU_HAS_SUBS; strcpy (menus[SHARP_ZQ_MENU].option[3], "ZQ-4XXX");  
   menus[SHARP_ZQ_MENU].attrib[4] = MENU_HAS_SUBS; strcpy (menus[SHARP_ZQ_MENU].option[4], "ZQ-5XXX");
   menus[SHARP_ZQ_MENU].attrib[5] = MENU_HAS_SUBS; strcpy (menus[SHARP_ZQ_MENU].option[5], "ZQ-6XXX");
   
   menus[EL6XXX_MENU].start_x = menus[SHARP_EL_MENU].start_x+menus[SHARP_EL_MENU].width+3;
   menus[EL6XXX_MENU].start_y = menus[SHARP_EL_MENU].start_y+1;
   menus[EL6XXX_MENU].width = 11; menus[EL6XXX_MENU].number_options = 7;
   menus[EL6XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[0], "EL-6000");
   menus[EL6XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[1], "EL-6170");
   menus[EL6XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[2], "EL-6190");
   menus[EL6XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[3], "EL-6320");
   menus[EL6XXX_MENU].attrib[4] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[4], "EL-6330");
   menus[EL6XXX_MENU].attrib[5] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[5], "EL-6360");
   menus[EL6XXX_MENU].attrib[6] = MENU_NORMAL; strcpy (menus[EL6XXX_MENU].option[6], "EL-6390");

   menus[IQ7XXX_MENU].start_x = menus[SHARP_IQ_MENU].start_x+menus[SHARP_IQ_MENU].width+3;
   menus[IQ7XXX_MENU].start_y = menus[SHARP_IQ_MENU].start_y+1;
   menus[IQ7XXX_MENU].width = 11; menus[IQ7XXX_MENU].number_options = 9;
   menus[IQ7XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[0], "IQ-7000");
   menus[IQ7XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[1], "IQ-7100M");
   menus[IQ7XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[2], "IQ-7200");
   menus[IQ7XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[3], "IQ-7300M");
   menus[IQ7XXX_MENU].attrib[4] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[4], "IQ-7400");
   menus[IQ7XXX_MENU].attrib[5] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[5], "IQ-7420");
   menus[IQ7XXX_MENU].attrib[6] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[6], "IQ-7600");
   menus[IQ7XXX_MENU].attrib[7] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[7], "IQ-7620");
   menus[IQ7XXX_MENU].attrib[8] = MENU_NORMAL; strcpy (menus[IQ7XXX_MENU].option[8], "IQ-7690");

   menus[IQ8XXX_MENU].start_x = menus[SHARP_IQ_MENU].start_x+menus[SHARP_IQ_MENU].width+3;
   menus[IQ8XXX_MENU].start_y = menus[SHARP_IQ_MENU].start_y+2;
   menus[IQ8XXX_MENU].width = 11; menus[IQ8XXX_MENU].number_options = 9;
   menus[IQ8XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[0], "IQ-8000");
   menus[IQ8XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[1], "IQ-8100M");
   menus[IQ8XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[2], "IQ-8200");
   menus[IQ8XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[3], "IQ-8300M");
   menus[IQ8XXX_MENU].attrib[4] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[4], "IQ-8400");
   menus[IQ8XXX_MENU].attrib[5] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[5], "IQ-8500M");
   menus[IQ8XXX_MENU].attrib[6] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[6], "IQ-8600M");
   menus[IQ8XXX_MENU].attrib[7] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[7], "IQ-8900");
   menus[IQ8XXX_MENU].attrib[8] = MENU_NORMAL; strcpy (menus[IQ8XXX_MENU].option[8], "IQ-8920");

   menus[IQ9XXX_MENU].start_x = menus[SHARP_IQ_MENU].start_x+menus[SHARP_IQ_MENU].width+3;
   menus[IQ9XXX_MENU].start_y = menus[SHARP_IQ_MENU].start_y+3;
   menus[IQ9XXX_MENU].width = 15; menus[IQ9XXX_MENU].number_options = 4;
   menus[IQ9XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[IQ9XXX_MENU].option[0], "IQ-9000");
   menus[IQ9XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[IQ9XXX_MENU].option[1], "IQ-9000 MK2");
   menus[IQ9XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[IQ9XXX_MENU].option[2], "IQ-9200");
   menus[IQ9XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[IQ9XXX_MENU].option[3], "IQ-9200 MK2");

   menus[ZQ1XXX_MENU].start_x = menus[SHARP_ZQ_MENU].start_x+menus[SHARP_ZQ_MENU].width+3;
   menus[ZQ1XXX_MENU].start_y = menus[SHARP_ZQ_MENU].start_y+1;
   menus[ZQ1XXX_MENU].width = 12; menus[ZQ1XXX_MENU].number_options = 4;
   menus[ZQ1XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[ZQ1XXX_MENU].option[0], "ZQ-1000");
   menus[ZQ1XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[ZQ1XXX_MENU].option[1], "ZQ-1050");
   menus[ZQ1XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[ZQ1XXX_MENU].option[2], "ZQ-1200");
   menus[ZQ1XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[ZQ1XXX_MENU].option[3], "ZQ-1250A");

   menus[ZQ2XXX_MENU].start_x = menus[SHARP_ZQ_MENU].start_x+menus[SHARP_ZQ_MENU].width+3;
   menus[ZQ2XXX_MENU].start_y = menus[SHARP_ZQ_MENU].start_y+2;
   menus[ZQ2XXX_MENU].width = 11; menus[ZQ2XXX_MENU].number_options = 7;
   menus[ZQ2XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[ZQ2XXX_MENU].option[0], "ZQ-2200");
   menus[ZQ2XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[ZQ2XXX_MENU].option[1], "ZQ-2250");
   menus[ZQ2XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[ZQ2XXX_MENU].option[2], "ZQ-2400");
   menus[ZQ2XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[ZQ2XXX_MENU].option[3], "ZQ-2450");
   menus[ZQ2XXX_MENU].attrib[4] = MENU_GREYED; strcpy (menus[ZQ2XXX_MENU].option[4], "ZQ-2500");
   menus[ZQ2XXX_MENU].attrib[5] = MENU_GREYED; strcpy (menus[ZQ2XXX_MENU].option[5], "ZQ-2700");
   menus[ZQ2XXX_MENU].attrib[6] = MENU_GREYED; strcpy (menus[ZQ2XXX_MENU].option[6], "ZQ-2750");

   menus[ZQ3XXX_MENU].start_x = menus[SHARP_ZQ_MENU].start_x+menus[SHARP_ZQ_MENU].width+3;
   menus[ZQ3XXX_MENU].start_y = menus[SHARP_ZQ_MENU].start_y+3;
   menus[ZQ3XXX_MENU].width = 11; menus[ZQ3XXX_MENU].number_options = 3;
   menus[ZQ3XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[ZQ3XXX_MENU].option[0], "ZQ-3000");
   menus[ZQ3XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[ZQ3XXX_MENU].option[1], "ZQ-3200");
   menus[ZQ3XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[ZQ3XXX_MENU].option[2], "ZQ-3250");

   menus[ZQ4XXX_MENU].start_x = menus[SHARP_ZQ_MENU].start_x+menus[SHARP_ZQ_MENU].width+3;
   menus[ZQ4XXX_MENU].start_y = menus[SHARP_ZQ_MENU].start_y+4;
   menus[ZQ4XXX_MENU].width = 11; menus[ZQ4XXX_MENU].number_options = 1;
   menus[ZQ4XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[ZQ4XXX_MENU].option[0], "ZQ-4450");

   menus[ZQ5XXX_MENU].start_x = menus[SHARP_ZQ_MENU].start_x+menus[SHARP_ZQ_MENU].width+3;
   menus[ZQ5XXX_MENU].start_y = menus[SHARP_ZQ_MENU].start_y+5;
   menus[ZQ5XXX_MENU].width = 11; menus[ZQ5XXX_MENU].number_options = 5;
   menus[ZQ5XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[ZQ5XXX_MENU].option[0], "ZQ-5000");
   menus[ZQ5XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[ZQ5XXX_MENU].option[1], "ZQ-5100M");
   menus[ZQ5XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[ZQ5XXX_MENU].option[2], "ZQ-5200");
   menus[ZQ5XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[ZQ5XXX_MENU].option[3], "ZQ-5300");
   menus[ZQ5XXX_MENU].attrib[4] = MENU_NORMAL; strcpy (menus[ZQ5XXX_MENU].option[4], "ZQ-5300M");

   menus[ZQ6XXX_MENU].start_x = menus[SHARP_ZQ_MENU].start_x+menus[SHARP_ZQ_MENU].width+3;
   menus[ZQ6XXX_MENU].start_y = menus[SHARP_ZQ_MENU].start_y+6;
   menus[ZQ6XXX_MENU].width = 11; menus[ZQ6XXX_MENU].number_options = 4;
   menus[ZQ6XXX_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[ZQ6XXX_MENU].option[0], "ZQ-6000");
   menus[ZQ6XXX_MENU].attrib[1] = MENU_NORMAL; strcpy (menus[ZQ6XXX_MENU].option[1], "ZQ-6100M");
   menus[ZQ6XXX_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[ZQ6XXX_MENU].option[2], "ZQ-6200");
   menus[ZQ6XXX_MENU].attrib[3] = MENU_NORMAL; strcpy (menus[ZQ6XXX_MENU].option[3], "ZQ-6300M");

   /* Exit Menu */

   menus[EXIT_MENU].start_x = 43; menus[EXIT_MENU].start_y = 2;
   menus[EXIT_MENU].width = 12; menus[EXIT_MENU].number_options = 3;
   menus[EXIT_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[EXIT_MENU].option[0], "Shell To OS");
   menus[EXIT_MENU].attrib[1] = MENU_SEPARATOR; strcpy (menus[EXIT_MENU].option[1], "");
   menus[EXIT_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[EXIT_MENU].option[2], "Leave IQuest");
   menu_table[EXIT_MENU][3] = LEAVE_IQUEST;
   menu_table[EXIT_MENU][1] = DOS_SHELL;

   /* Help Menu */

   menus[HELP_MENU].start_x = 67; menus[HELP_MENU].start_y = 2;
   menus[HELP_MENU].width = 9; menus[HELP_MENU].number_options = 3;
   menus[HELP_MENU].attrib[0] = MENU_NORMAL; strcpy (menus[HELP_MENU].option[0], "Glossary");
   menus[HELP_MENU].attrib[1] = MENU_SEPARATOR; strcpy (menus[HELP_MENU].option[1], "");
   menus[HELP_MENU].attrib[2] = MENU_NORMAL; strcpy (menus[HELP_MENU].option[2], "About");

   menu_table[HELP_MENU][3] = ABOUT_IQUEST;
}

void highlight_option ( int menu_number, int option, int state )
{
   if (state)
      SetColourPair (palette.bright_white, palette.black);
   else
     switch ( menus[menu_number].attrib[option])
     {
        case MENU_NORMAL   :
        case MENU_HAS_SUBS : SetColourPair (palette.yellow, palette.blue);
                             break;
        case MENU_GREYED   : SetColourPair (palette.white, palette.blue);
                             break;
     }
   WriteString (menus[menu_number].start_x+2, menus[menu_number].start_y+1+option,
                menus[menu_number].option[option]);
}


void draw_menu (int x1, int y1, int x2, int y2, int menu_number, int state)
{
   int i;

     if (state)
       SetColourPair (palette.bright_white, palette.blue);
     else
       SetColourPair (palette.white, palette.blue);
     
     draw_window (x1,y1,x2,y2);

     for (i=0; i < menus[menu_number].number_options; i++)
     {
       switch(menus[menu_number].attrib[i])
       {
          case MENU_NORMAL    : 
                                SetColourPair (palette.yellow, palette.blue);
                                WriteString (x1+2, y1+1+i, menus[menu_number].option[i]);
                                break;
          case MENU_GREYED    : SetColourPair (palette.white, palette.blue);
                                WriteString (x1+2, y1+i+1, menus[menu_number].option[i]);
                                break;
          case MENU_SEPARATOR : 
                                if (state)
                                  SetColourPair (palette.bright_white, palette.blue);
                                else
                                  SetColourPair (palette.white, palette.blue);
                                switch ( config_byte1 & MSK_BORDER_TYPE)
                                {
                                  case MSK_SINGLE_LINE_BORDER : WriteChar (x1, y1+i+1, (char)195);
                                                                WriteChar (x1+menus[menu_number].width+2, y1+i+1, (char)180);
                                                                WriteMultiChar (x1+1, y1+i+1, (char)196, menus[menu_number].width+1);
                                                                break;
                                  case MSK_DOUBLE_LINE_BORDER : WriteChar ( x1, y1+i+1, (char)204);
                                                                WriteChar ( x1 + menus[menu_number].width+2, y1+i+1, (char)185);
                                                                WriteMultiChar (x1+1, y1+i+1, (char)205, menus[menu_number].width+1);
                                                                break;
                                  default                     :
                                                                break;
                                }
                                break;
          case MENU_HAS_SUBS  : SetColourPair (palette.yellow, palette.blue);
                                WriteString (x1+2, y1+1+i, menus[menu_number].option[i]);
                                SetColourPair (palette.red, palette.blue);
                                WriteChar (x1+menus[menu_number].width+1, y1+i+1, SUBDIR_MARKER);
       }
     }
}

void open_menu (int menu_number, int layer)
{
   int x1, y1, x2, y2;

   if ((menu_number == FILE_MENU) &&
       (file_list_count == 0)
      )
   {
      menus[FILE_MENU].attrib[4] = MENU_GREYED;            
      menu_table[FILE_MENU][5] = -999;
   }
   else
   {
      menus[FILE_MENU].attrib[4] = MENU_NORMAL;
      menu_table[FILE_MENU][5] = PREVIOUS_FILES;
   }

   menu_option = 0;
   x1 = menus[menu_number].start_x;
   y1 = menus[menu_number].start_y;
   x2 = menus[menu_number].start_x+menus[menu_number].width+2;
   y2 = menus[menu_number].start_y+menus[menu_number].number_options+1;
   menu_buff[layer] = (char *)malloc (PORTSIZE(x1,y1,x2+1,y2+1));
   if (menu_buff[layer] != NULL)
   {
     save_area (x1,y1,x2+1,y2+1, menu_buff[layer]);
     SetColourPair (palette.bright_white, palette.blue);
     draw_menu (x1, y1, x2, y2, menu_number, TRUE);
   }
   highlight_option (menu_number, 0, TRUE);
}

void close_menu ( int menu_number, int layer )
{
   int x1, y1, x2, y2;


   x1 = menus[menu_number].start_x;
   y1 = menus[menu_number].start_y;
   x2 = menus[menu_number].start_x + menus[menu_number].width+2;
   y2 = menus[menu_number].start_y + menus[menu_number].number_options+1;
   restore_area (x1,y1,x2+1,y2+1, menu_buff[layer]);
   free ((char *)menu_buff[layer]);
   menu_option = -1;


}

void shrink_menus ( int layer )
{
   int level;

   level = layer;
   while ( level != 0 )
   {
      close_menu (current_menu, level);
      current_menu = menu_table[current_menu][0];
      level--;
   }
   last_menu = current_menu;
   close_menu(current_menu, 0);

   current_menu =-1;
   current_level =-1;
                 
   show_menu_bar ();
}


