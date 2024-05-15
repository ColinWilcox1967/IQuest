#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include "errlist.h"
#include "iq.h"
#include "comm.h"
#include "palette.h"
#include "windows.h"
#include "gf.h"
#include "asiports.h"
#include "trans.h"
#include "iqcomms.h"

#define ESC      27
#define ENQ      5
#define SYN      22

char *iq9000_buff;

extern int number_of_modes;
extern char mode_buffer[];
extern struct comms_def comms;
extern int model;
extern char current_organiser[];
extern struct palette_str palette;

extern int get_key ( void );
extern void centre_push_button(int x1, int x2, int y, char msg[]);
extern int check_rx_buf (int port);
extern void create_dynamic_menu(void);
extern void comms_error (int errnum);
extern int  warning (void);
extern void no_serial_comms(char type[]);
extern int  serial_model (int type);
extern void show_error (int errnum);
extern void SetColourPair (short fg, short bk);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern void delay (int n);
extern void break_down_mode_buffer(void);
extern void show_iq9000_comms_error (int errcode);
extern int IQ_Connect ( int port );
extern void IQ_Disconnect ( int port );

void open_iq9000_connect_box (void);
int  get_iq9000_dir (void);
void close_iq9000_connect_box (void);
void decode_iq9000_dir (void);

void iq9000_transfer (int port);

void open_iq9000_connect_box()
{
   iq9000_buff = (char *)malloc(PORTSIZE(20, 8, 61, 14));
   if (iq9000_buff  != NULL)
   {
      save_area (20,8,61,14,iq9000_buff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (20,8,60,13);
      SetColourPair (palette.yellow, palette.blue);
      centre_text (21,59,10,"Requesting Mode List From Organiser");
      centre_text (21,59,11, "** PLEASE WAIT **");
   }
}

void close_iq9000_connect_box()
{
   if (iq9000_buff != NULL)
   {
      restore_area (20,8,61,14,iq9000_buff);
      free((char *)iq9000_buff);
   }
}

void iq9000_transfer(int port)
{
   int status;

   if (serial_model (model))
   {
      if (!warning())
         return;
      
      open_iq9000_connect_box();
      status = IQ_Connect (port);
      
      if (status != IQ_ERROR ) // Init Done OK
      {
         // connection established 
         // Request Mode List From Organiser

         status = get_iq9000_dir(port);
         close_iq9000_connect_box();
         if (status == 0) // got dir ok !!
             create_dynamic_menu();
         else
            show_iq9000_comms_error ( IQ9000_NO_DIR );
      }
      else
      {   
          close_iq9000_connect_box();
          show_iq9000_comms_error (IQ9000_NO_CONNECT);
      }
      IQ_Disconnect(port);
   }
   else
     no_serial_comms (current_organiser);
}
