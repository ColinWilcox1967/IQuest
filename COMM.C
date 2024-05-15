#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

#include "config.h"
#include "windows.h"
#include "keys.h"
#include "palette.h"
#include "iq.h"
#include "comm.h"

char *comm_port[4] = {"COM1", "COM2","COM3","COM4"};
char *parity[3] = {"Odd", "Even", "None"};
char *baud_rate[5] = {"300","1200","2400","4800","9600"};
char *data_bits[2] = {"7","8"};
char *stop_bits[2] = {"1","2"};
char *flow_control[2] = {"Hardware", "XON/XOFF"};

extern struct palette_str palette;

extern void WriteString  (int x, int y, char str[]);
extern int get_yesno (int x, int y, short fg, short bk);
extern unsigned int get_key( void);
extern void save_area (int x1, int y1, int x2, int y2, char *b);
extern void restore_area (int x1, int y1, int x2, int y2, char *b);
extern void draw_window (int x1, int y1, int x2, int y2);
extern void SetColourPair (short fg, short bk);
extern unsigned int draw_button ( int x, int y, int width, char msg[],int state);
extern void window_title ( int x1, int x2, int y, char msg[], short fg, short bk);
extern void simulate_push (int x, int y, char msg[], int width);

int comms_setup=FALSE;


struct comms_def comms;

void set_default_comms ( void );
void do_serial_comms ( void );
void show_comm_header (int num, int state);
void show_comm_options (int num, int state);
void show_present_comms_settings ( void );
void highlight_option ( int option, int state);
void set_comm_options ( void );
int  keep_in_range ( int option, int choice);
int  current_option ( int option );
int abandon_serial_port_changes ( void );

void set_default_comms ()
{
  comms_setup = TRUE;
  comms.port = PORT1;
  comms.parity = PARITY_NONE;
  comms.stop = STOP_BITS1;
  comms.speed = BAUD_9600;
  comms.data = DATA_BITS8;
  comms.flow = FLOW_XONXOFF;
}


void show_comm_options (int option, int state)
{
  int choice;

  if (state)
     SetColourPair (palette.bright_white, palette.black);
  else
     SetColourPair (palette.black, palette.green);
  switch ( option)
  {
     case 1 : 
              for (choice = 0; choice < 4; choice++)
                 WriteString (30+(choice*7), 9, comm_port[choice]);
              break;
     
     case 2 : for (choice = 0; choice < 5; choice++)
                WriteString (30+(choice * 7), 11, baud_rate[choice]);
              break;
     case 3 : for (choice = 0; choice < 3; choice++)
                WriteString (30+(choice*7),12, parity[choice]);
              break;
     case 4 : for (choice = 0; choice < 2; choice++)
                WriteString (30+(choice*3), 13, stop_bits[choice]);
              break;
     case 5 : for (choice = 0; choice < 2; choice++)
                WriteString (30+(choice*3),14, data_bits[choice]);
              break;
     case 6 : for (choice = 0; choice < 2; choice++)
                 WriteString (30+(choice*10), 15, flow_control[choice]);
              break;
     default : break;
  }
}

void show_comm_header ( int n, int state)
{
  if (state)
     SetColourPair (palette.bright_white, palette.black);
  else
     SetColourPair (palette.black, palette.green);
  switch (n)
  {
     case 1 : WriteString (15, 9, "Port");
              break;
     case 2 : WriteString (15, 11, "Speed"); 
              break;
     case 3 : WriteString(15,12, "Parity");
              break;
     case 4 : WriteString (15,13, "Stop Bits");
              break;
     case 5 : WriteString (15,14, "Data Bits");
              break;
     case 6 : WriteString (15,15, "Flow Control");
              break;
  }
  if (state)
    SetColourPair (palette.red, palette.black);
  else
    SetColourPair (palette.red, palette.green);
  switch (n )
  {
    case 1  : WriteChar (15, 9, 'P');
              break;
    case 2  : WriteChar (15,11, 'S');
              break;
    case 3  : WriteChar (16,12, 'a');
              break;
    case 4  : WriteChar (16,13,'t');
              break;
    case 5  : WriteChar (15, 14, 'D');
              break;
    case 6  : WriteChar (15, 15, 'F');
              break;   
    default : break;
  }
}

void highlight_comm_option (int option, int state)
{
  if (state)
    SetColourPair (palette.bright_white, palette.black);
  else
    SetColourPair (palette.black, palette.green);
  switch ( option )
  {
     case 1 : /* Port Setting */
               WriteString (30 + ( 7 * comms.port), 9, comm_port[comms.port]);
               break;
     case 2 : /* Baud Rate */
               WriteString (30+ ( 7 * comms.speed), 11, baud_rate[comms.speed]);
               break;
     case 3 : /* Parity */
               WriteString (30+(7 * comms.parity), 12, parity[comms.parity]);
               break;
     case 4 : /* Data Bits */
               WriteString (30 + (3 * comms.stop), 13, stop_bits[comms.stop]);
               break;
     case 5 : /* Stop Bits */
               WriteString (30 + (3 * comms.data), 14, data_bits[comms.data]);
               break;
     case 6 : /* Flow Control */
               WriteString (30+(comms.flow * 10), 15, flow_control[comms.flow]);
               break;
     default : break;
  }
}

void show_present_comms_settings ()
{
  int i;

  for (i=1; i <=6; i++)
    highlight_comm_option (i, TRUE);
}

int current_choice ( int option )
{
   switch ( option )
   {
      case 1 : switch ( comms.port)
               {
                   case PORT1 : return(0);
                   case PORT2 : return(1);
                   case PORT3 : return(2);
                   case PORT4 : return(3);
               }
               break;
      case 2 : switch ( comms.speed)
               {
                   case BAUD_300   : return(0);
                   case BAUD_1200  : return(1);
                   case BAUD_2400  : return(2);
                   case BAUD_4800  : return(3);
                   case BAUD_9600  : return(4);
                }
                break;
      case 3 : switch ( comms.parity)
               {
                  case PARITY_ODD  : return(0);
                  case PARITY_EVEN : return(1);
                  case PARITY_NONE : return(2);
               }
               break;
      case 4 : switch ( comms.stop)
               {
                   case STOP_BITS1 : return(0);
                   case STOP_BITS2 : return(1);
               }
               break;
      case 5 : switch ( comms.data)
               {
                   case DATA_BITS7 : return(0);
                   case DATA_BITS8 : return(1);
               }
               break;
      case 6 : switch ( comms.flow)
               {
                   case FLOW_HARDWARE : return(0);
                   case FLOW_XONXOFF  : return(1);
               }
               break;
      default : return(-1);
   }
}

int keep_in_range ( int option, int choice)
{
   int answer;

   answer = choice;
   switch ( option )
   {
      case 1 : 
               if ( choice < PORT1)
                    answer = PORT4;
               if ( choice > PORT4)
                  answer = PORT1;
               comms.port = answer;
               break;
      case 2 : if (choice < BAUD_300)
                 answer = BAUD_9600;
               if (choice > BAUD_9600)
                  answer = BAUD_300;
               comms.speed = answer;
               break;
      case 3 : if ( choice < PARITY_ODD)
                   answer = PARITY_NONE;
               if ( choice > PARITY_NONE)
                  answer = PARITY_ODD;
               comms.parity = answer;
               break;
      case 4 : if ( choice < STOP_BITS1)
                  answer = STOP_BITS2;
               if (choice > STOP_BITS2)
                   answer = STOP_BITS1;
               comms.stop = answer;
               break;
      case 5 : if ( choice < DATA_BITS7)
                  answer = DATA_BITS8;
               if ( choice > DATA_BITS8)
                   answer = DATA_BITS7;
               comms.data = answer;
               break;
      case 6 : if (choice < FLOW_HARDWARE)
                  answer = FLOW_XONXOFF;
               if (choice > FLOW_XONXOFF)
                 answer =  FLOW_HARDWARE;
               comms.flow = answer;
               break;
      default : 
                break;
   }
   return(answer);
}


int abandon_serial_port_changes ()
{
   char *buff;
   int  answer;

   answer = FALSE;
   buff = (char *)malloc(PORTSIZE(25, 9, 56, 13));
   if (buff != NULL)
   {
      save_area (25, 9, 56, 13, buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window (25, 9, 55,12);
      SetColourPair (palette.yellow, palette.red);
      WriteString (29, 10, "Abandon Comms Settings ?");
      answer = get_yesno ( 34, 11, palette.yellow, palette.red);
      restore_area (25, 9, 56, 13, buff);
      free ((char *)buff);
   }
   return (answer);

}

void set_comm_options ()
{
  int              option;
  int              finished = FALSE;
  int              choice;
  unsigned int     key;
  struct comms_def old_settings;

  memcpy (&old_settings.port, &comms.port, sizeof (struct comms_def));
  option = 1; /* start on comm port */
  while (! finished )
  {
      show_comm_header ( option, TRUE);
      highlight_comm_option ( option, TRUE);
      key = get_key();
      show_comm_header ( option, FALSE);
      if ((key != K_CSR_UP) && ( key != K_CSR_DN) && 
          (key != 'O') && (key != K_CR) &&
          (key != 'C') && (key != K_ESC)  &&
          (key != 'c') && (key != 'o')
         )
          highlight_comm_option ( option, FALSE);
      switch ( key )
      {
          case K_CR     : 
          case 'O'      :
          case 'o'      :
                          finished = TRUE;
                          simulate_push (28,17, "&Ok", 4);
                          break;
          case K_ESC    : 
          case 'C'      : 
          case 'c'      :
                          finished = TRUE;
                          simulate_push (45, 17, "&Cancel", 8);
                          if (memcmp (&comms.port, &old_settings.port, sizeof (struct comms_def)) != 0)
                          {
                             if (abandon_serial_port_changes())
                                memcpy ( &comms.port, &old_settings.port, sizeof (struct comms_def));
                          }
                          break;
          case K_CSR_LT : choice--;
                          break;
          case K_CSR_RT : choice++;
                          break;
          case K_CSR_DN : option++;
                          if (option > 6)
                             option = 1;
                          choice = current_choice(option);
                          break;
          case K_CSR_UP : option--;
                          if (option < 1)
                             option = 6;
                          choice = current_choice(option);
                          break;
          case 'P'      : 
          case K_ALT_P  : 
          case 'p'      : option = 1;
                          choice = current_choice(option);
                          break;
          case 'S'      : 
          case K_ALT_S  : 
          case 's'      : option = 2;
                          choice = current_choice(option);
                          break;
          case 'A'      :
          case K_ALT_A  : 
          case 'a'      : option = 3;
                          choice = current_choice(option);
                          break;
          case 'T'      :
          case K_ALT_T  : 
          case 't'      : option = 4;
                          choice = current_choice(option);
                          break;
          case 'D'      : 
          case K_ALT_D  : 
          case 'd'      : option = 5;
                          choice = current_choice(option);
                          break;
          case 'F'      : 
          case K_ALT_F  : 
          case 'f'      : option = 6;
                          choice = current_choice(option);
                          break;
          default       : break;
      }

      switch (option)
      {
          case 1 : choice = keep_in_range ( option, choice);
                   break;
          case 2 : choice = keep_in_range (option, choice);
                   break;
          case 3 : choice = keep_in_range ( option, choice);
                   break;
          case 4 : choice = keep_in_range ( option, choice);
                   break;
          case 5 : choice = keep_in_range ( option, choice);
                   break;
          case 6 : choice = keep_in_range ( option, choice);
                   break;
      }
      show_comm_header ( option, TRUE);
      highlight_comm_option ( option, TRUE);
   }   
}

void do_serial_comms ( void )
{
   char *buff;
   int option;

   if (! comms_setup)
     set_default_comms();

   buff = (char *)malloc(PORTSIZE(12, 7, 69, 19));
   if (buff != NULL)
   {
      save_area (12, 7, 69, 19, buff);
      SetColourPair (palette.black, palette.green);
      draw_window (12, 7, 68, 18);
      for (option = 1; option <= 6; option++)
      {
         show_comm_header (option, FALSE);  
         show_comm_options ( option, FALSE);
      }
      window_title ( 12, 68, 7, "Serial Communications Settings", palette.red, palette.green);
      show_present_comms_settings();
      draw_button (28, 17, 4, "&Ok", BUTTON_UP);
      draw_button (45, 17, 8, "&Cancel", BUTTON_UP);
      set_comm_options();
      restore_area (12, 7, 69, 19, buff);
      free ((char *)buff);
   }
}


