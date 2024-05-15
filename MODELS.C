#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu.h"
#include "config.h"
#include "palette.h"
#include "windows.h"
#include "comm.h"

void process_iq_models ( int model );
void process_zq_models ( int model );
void process_el_models ( int model);
void process_xe_models ( int model );
void show_model_name ( char model[]);

extern void WriteMultiChar (int x, int y, char ch, int n);
extern void SetColourPair (short fg, short bk);
extern void shrink_menus ( int level );
extern void window_title ( int x1, int x2, int y, char msg[], int fg, int bk);
extern void show_menu_bar ( void );

extern unsigned char config_byte1;
extern struct comms_def comms;
extern struct palette_str palette;
extern char current_organiser[];
extern int current_level;


void show_model_name ( char model[])
{
   char str[80];

   SetColourPair (palette.red,  palette.cyan);
   if (comms.port == -1)                                                   
      sprintf (str, "Current Model : %s On NONE", model);
   else
      sprintf (str, "Current Model : %s On COM%d", model, comms.port+1);
   SetColourPair (palette.black, palette.cyan);
   if (config_byte1 & MSK_BORDER_TYPE)
     WriteMultiChar (5, 4, (char)H_SINGLE, 70);
   else
     WriteMultiChar (5, 4, (char)H_DOUBLE, 70);
   window_title (3, 77, 4, str, palette.red, palette.cyan);   
   show_menu_bar ();
}

void process_iq_models ( int model )
{
    shrink_menus ( current_level );
    switch (model) 
    {
       case IQ_7100M: strcpy (current_organiser, "Sharp IQ-7100M");
                      break;
       case IQ_7300M: strcpy (current_organiser, "Sharp IQ-7300M");
                      break;
       case IQ_7000 : strcpy (current_organiser, "Sharp IQ-7000");
                      break;
       case IQ_7200 : strcpy (current_organiser, "Sharp IQ-7200");
                      break;
       case IQ_7400 : strcpy (current_organiser, "Sharp IQ-7400");
                      break;
       case IQ_7420 : strcpy (current_organiser, "Sharp IQ-7420");
                      break;
       case IQ_7600 : strcpy (current_organiser, "Sharp IQ-7600");
                      break;
       case IQ_7620 : strcpy (current_organiser, "Sharp IQ-7620");
                      break;
       case IQ_7690 : strcpy (current_organiser, "Sharp IQ-7690");
                      break;
       case IQ_8000 : strcpy (current_organiser, "Sharp IQ-8000");
                      break;
       case IQ_8100M: strcpy (current_organiser, "Sharp IQ-8100M");
                      break;
       case IQ_8200 : strcpy (current_organiser, "Sharp IQ-8200");
                      break;
       case IQ_8300M: strcpy (current_organiser, "Sharp IQ-8300M");
                      break;
       case IQ_8400 : strcpy (current_organiser, "Sharp IQ-8400");
                      break;
       case IQ_8500M : strcpy (current_organiser, "Sharp IQ-8500M");
                       break;
       case IQ_8600M : strcpy (current_organiser, "Sharp IQ-8600M");
                       break;
       case IQ_8900 : strcpy (current_organiser, "Sharp IQ-8900");
                      break;
       case IQ_8920 : strcpy (current_organiser, "Sharp IQ-8920");
                      break;
       case IQ_9000    : strcpy (current_organiser, "Sharp IQ-9000");
                         break;
       case IQ_9000MK2 : strcpy (current_organiser, "Sharp IQ-9000 MK2");
                         break;
       case IQ_9200    : strcpy (current_organiser, "Sharp IQ-9200");
                         break;
       case IQ_9200MK2 : strcpy (current_organiser, "Sharp IQ-9200 MK2");
                         break;
       default      : break; 
    }
    show_model_name(current_organiser);
}

void process_xe_models ( int model )
{
   shrink_menus (current_level);
   switch (model )
   {
       case XE_7000 : strcpy (current_organiser, "Sharp Xeraus");
                      break;
       default      : break;
   }
   show_model_name(current_organiser);
}

void process_zq_models ( int model )
{
    shrink_menus( current_level);
    switch ( model)
    {
       case ZQ_1000 : strcpy (current_organiser, "Sharp ZQ-1000");
                      break;
       case ZQ_1050 : strcpy (current_organiser, "Sharp ZQ-1050");
                      break;
       case ZQ_1200 : strcpy (current_organiser, "Sharp ZQ-1200");
                      break;
       case ZQ_1250A : strcpy (current_organiser, "Sharp ZQ-1250A");
                      break;
       case ZQ_2200 : strcpy (current_organiser, "Sharp ZQ-2200");
                      break;
       case ZQ_2250 : strcpy (current_organiser, "Sharp ZQ-2250");
                      break;
       case ZQ_2400 : strcpy (current_organiser, "Sharp ZQ-2400");
                      break;
       case ZQ_2450 : strcpy (current_organiser, "Sharp ZQ-2450");
                      break;
       case ZQ_2500 : strcpy (current_organiser, "Sharp ZQ-2500");
                      break;
       case ZQ_2700 : strcpy (current_organiser, "Sharp ZQ-2700");
                      break;
       case ZQ_2750 : strcpy (current_organiser, "Sharp ZQ-2750");
                      break;
       case ZQ_3000 : strcpy (current_organiser, "Sharp ZQ-3000");
                      break;
       case ZQ_3200 : strcpy (current_organiser, "Sharp ZQ-3200");
                      break;
       case ZQ_3250 : strcpy (current_organiser, "Sharp ZQ-3250");
                      break;
       case ZQ_4450 : strcpy (current_organiser, "Sharp ZQ-4450");
                      break;
       case ZQ_5000 : strcpy (current_organiser, "Sharp ZQ-5000");
                      break;
       case ZQ_5100M : strcpy (current_organiser, "Sharp ZQ-5100M");
                      break;
       case ZQ_5200 : strcpy (current_organiser, "Sharp ZQ-5200");
                      break;
       case ZQ_5300M : strcpy (current_organiser, "Sharp ZQ-5300M");
                      break;
       case ZQ_6000 : strcpy (current_organiser, "Sharp ZQ-6000");
                      break;
       case ZQ_6100M : strcpy (current_organiser, "Sharp ZQ-6100M");
                      break;
       case ZQ_6200 : strcpy (current_organiser, "Sharp ZQ-6200");
                      break;
       case ZQ_6300M : strcpy (current_organiser, "Sharp ZQ-6300M");
                      break;

       default      : break;
    }
    show_model_name(current_organiser);
}

void process_el_models ( int model )
{
   shrink_menus ( current_level);
   switch ( model )
   {
       case EL_6000 : strcpy (current_organiser, "Sharp EL-6000");
                      break;
       case EL_6170 : strcpy (current_organiser, "Sharp EL-6170");
                      break;
       case EL_6190 : strcpy (current_organiser, "Sharp EL-6190");
                      break;
       case EL_6320 : strcpy (current_organiser, "Sharp EL-6320");
                      break;
       case EL_6330 : strcpy (current_organiser, "Sharp EL-6330");
                      break;
       case EL_6360 : strcpy (current_organiser, "Sharp EL-6360");
                      break;
       case EL_6390 : strcpy (current_organiser, "Sharp EL-6390");
                      break;

      default      : break;
   }
   show_model_name(current_organiser);
}

