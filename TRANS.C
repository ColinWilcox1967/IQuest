#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "gf.h"
#include "asiports.h"
#include "iq.h"
#include "trans.h"
#include "errlist.h"
#include "palette.h"
#include "keys.h"
#include "config.h"
#include "windows.h"
#include "menu.h"
#include "comm.h"
#include "iqcomms.h"

extern unsigned char config_byte1;
extern struct palette_str palette;
extern struct comms_def comms;
extern char current_organiser[];
extern char info_block1[];
extern int model;
extern int iq_type;
extern unsigned char config_byte2;

char *recv_buff;
char mode_buffer[1024];
int  window_size,
     number_of_modes;
int  data_transfer_mode;
int  tagged;
unsigned long chars_read;
int           block_count;

extern int IQ_Disconnect ( int port);
extern int IQ_Get_Blocks ( int port, char name[], int show_blocks);
extern void iq9000_transfer (int port);
extern void show_error (int errnum);
extern int abandon_option_changes(void);
extern int warning(void);
extern int IQtoPC (int iqtype, char name[], char filename[], int mode);
extern void no_serial_comms (char msg[]);
extern int recv_info_blocks (int type);
extern unsigned int get_key ( void );
extern void WriteMultiChar (int x, int y, char ch, int n);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void push_wait ( void );
extern unsigned int draw_button (int x, int y, int width, char msg[], int state);
extern void write_blanks (int x, int y, int n);
extern void simulate_push (int x, int y, char msg[], int width);
extern void centre_push_button ( int x1, int x2, int y, char msg[]);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern void scroll_up_area (int x1, int y1, int x2, int y2);
extern int IQconnect (int port, unsigned rxbufsize, unsigned txbufsize);
extern void IQterminate (int port, int type);
extern int IQ_Send_Command (int port, char *command);

int serial_model (int organiser_model);
void select_trans_mode ( void );
void show_trans_mode ( void );
void open_receive_statics ( char mode[] );
void close_receive_statics ( void );
int get_data ( int idx );
int receive_mode_data (int idx);
int do_receive ( int idx );
void write_receive_status ( char msg[]);
void break_down_mode_buffer ( void );
void show_mode_list ( int first, int last);
void get_tagged_modes( void );
void mode_help ( void );
void update_status_bar ( void );
void clear_status_bar ( void );
void create_dynamic_menu ( void );
void highlight_mode_name ( int start, int mode, int state);
int  tag_modes ( void );
void  show_mode (int start, int offset);
int request_directory ( void );
void data_transfer(void);
void comms_error ( int err );
void show_IQ8000_error_code (int code);
int get_organiser_type (int model);

void show_trans_mode ()
{
   SetColourPair (palette.bright_white, palette.green);
   WriteChar (30, 10, ' ');
   WriteChar (30, 11, ' ');
   if (config_byte2 & MSK_TRANS_MODE)
      WriteChar (30, 11, 'X');
   else
      WriteChar (30, 10, 'X');
}

void select_trans_mode ( void )
{
   char *buff;
   unsigned int key;
   int          finish,
                option;
   unsigned char old_config_byte2;

   buff = (char *)malloc(PORTSIZE (25, 8, 56, 15));
   if (buff != NULL)
   {
       save_area (25,8,56,15,buff);
       SetColourPair (palette.black, palette.green);
       draw_window (25,8,55,14);
       window_title (25,55,8,"Transfer Mode", palette.red, palette.green);
       SetColourPair (palette.black, palette.green);
       WriteString (28, 10, "[   ] Append");
       WriteString (28, 11, "[   ] Replace");
       SetColourPair (palette.red, palette.green);
       WriteChar (34, 10, 'A'); WriteChar (34, 11, 'R');
       draw_button (34, 13, 4, "&Ok", BUTTON_UP);
       draw_button (42, 13, 8, "&Cancel", BUTTON_UP);
       finish = FALSE;
       if (config_byte2 & MSK_TRANS_MODE)
          option = 2;
       else
          option=1;
       old_config_byte2 = config_byte2;
       while (! finish)
       {
           if (option == 1)
              config_byte2 &= ~MSK_TRANS_MODE;
           else
              config_byte2 |= MSK_TRANS_MODE;
           show_trans_mode ();
           key = get_key();
           switch (key)
           {
              case K_CSR_UP   : 
              case K_REVTAB   : option--;
                                if (option < 1) option=2;
                                break;
              case K_CSR_DN   :
              case K_TAB      : option++;
                                if (option>2) option=1;
                                break;
              case 'O'  :
              case K_CR : finish = TRUE;
                          simulate_push (34, 13, "&Ok", 4);
                          push_wait();
                          break;
              case 'C'   :
              case K_ESC : finish = TRUE;
                           simulate_push (42, 13, "&Cancel", 8);
                           if (config_byte2 != old_config_byte2)
                           {
                               if (abandon_option_changes())
                                  config_byte2 = old_config_byte2;
                           }
                           break;
              case 'A'  :
              case 'a'  : 
                          option=1;
                          break;
              case 'R'  :
              case 'r'  : 
                          option=2;
                          break;
              default   : break;
           }
       }
       restore_area (25,8,56,15, buff);
       free((char *)buff);
    }
}

int get_organiser_type ( int model )
{
   if ((model == IQ_8000) || (model == IQ_8200) || (model == IQ_8400))
      return(IQ8000);
   else
      return(IQ7000);
}

void show_IQ8000_error_code ( int code )
{
   char *buff;
   char msg[40];

   buff = (char *)malloc(PORTSIZE(20,8,61,14));
   if (buff != NULL)
   {
      save_area(20,8,61,14,buff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (20,8,60,13);
      SetColourPair (palette.yellow, palette.blue);
      switch (code)
      {
         case 0x41   : strcpy (msg, "I/O Device Error");
                       break;
         case 0x42   : strcpy (msg, "Memory Overflow");
                       break;
         case 0x43   : strcpy (msg, "Buffer Overflow");
                       break;
         case 0x44   : strcpy (msg, "Data Error");
                       break;
         case 0x46   : strcpy (msg, "Secret Mode Enabled");
                       break;
         case 0x48   : strcpy (msg, "Application Not Found");
                       break;
         case 0x49   : strcpy (msg, "Data Not Found");
                       break;
         case 0x4A   : strcpy (msg, "Data Modified");
                       break;
         case 0x4B   : strcpy (msg, "Command Not Supported");
                       break;
         case 0xFE   : strcpy (msg, "Battery Low Warning");
                       break;
         case 0xFF   : strcpy (msg, "ON /BREAK Key Pressed");
                       break;
         default     : sprintf (msg, "Unknown Status Byte : %d", code);
                       break;
      }
      centre_text (21, 60, 9, msg);
      centre_push_button (21, 60, 11, "&Ok"); 
      restore_area (20, 8, 61, 14, buff);
      free((char *)buff);
   }
}


void data_transfer ()
{
   if (iq_type == IQ9000 )
      iq9000_transfer(comms.port);
   else
   {
       if (serial_model(model))
       {
          if (!warning())
             return;
        
         if(request_directory())
         {
               memcpy (&mode_buffer[0], &info_block1[0], sizeof (mode_buffer));
               break_down_mode_buffer();
               create_dynamic_menu();
         }
      }
      else
        no_serial_comms (current_organiser);
   }
}


void comms_error (int err)
{
  char *buff;    
  char msg[40];
  
  buff = (char *)malloc(PORTSIZE(20,8,61,15));
  if (buff != NULL)
  {
     save_area (20,8,61,15,buff);
     SetColourPair (palette.bright_white, palette.red);
     draw_window (20,8,60,14);
     SetColourPair (palette.yellow, palette.red);
     centre_text (21,60,9,"** COMMS ERROR **");
     switch (err)
     {
        case IQ_OFFLINE     : strcpy (msg, "Organiser Is Offline/Switched Off");
                              break;
        case INVALID_PORT   : strcpy (msg, "Invalid Port Specified");
                              break;
        case RCV_TIMEOUT    : strcpy (msg, "Receive Timeout");
                              break;
        case CHECKSUM_ERROR : strcpy (msg, "Data Checksum Error");
                              break;
        case ASINVPORT      : strcpy (msg, "Invalid Port Specified");
                              break;
        case ASINUSE        : strcpy (msg, "Specified Port Already In Use");
                              break;
        case ASINVBUFSIZE   : strcpy (msg, "Invalid Buffer Size Specified");
                              break;
        case ASNOMEMORY     : strcpy (msg, "Insufficient Memory Free");
                              break;
        case ASNOTSETUP     : strcpy (msg, "Serial Port Not Initialised");
                              break;
        case ASINVPAR       : strcpy (msg, "Invalid Parameter Supplied");
                              break;
        case ASBUFREMPTY    : strcpy (msg, "Data Buffer Is Empty");
                              break;
        case ASBUFRFULL     : strcpy (msg, "Data Buffer Is Full");         
                              break;  
        case ASTIMEOUT      : strcpy (msg, "Serial Port Has Timed Out");
                              break;
        case ASNOCTS        : strcpy (msg, "Port Error : No CTS Found");
                              break;
        case ASNOCD         : strcpy (msg, "Port Error : No Carrier Found");
                              break;
        case ASNODSR        : strcpy (msg, "Port Error : No DSR Found");
                              break;
        case ASNO8250       : strcpy (msg, "Serious Error : No 8250 Serial Chip");
                              break;
        case ASXMSTATUS     : strcpy (msg, "Transmit Status Error");
                              break;
        case ASUSERABORT    : strcpy (msg, "User Has Aborted Process");
                              break;
        case ASFILERR       : strcpy (msg, "File Access Error");
                              break;
        case ASXMERROR      : strcpy (msg, "Data Transmission Error");
                              break;
        case ASNOWIDERX     : strcpy (msg, "Wide Reception Problem");
                              break;
        case ASCONFLICT     : strcpy (msg, "Serial Port Conflict");
                              break;
        default             : sprintf (msg, "Unknown Error : %d", err);
                              break;
     }
     centre_text (21,60,11,msg);
     centre_push_button (21,60,13, "&Continue");
     restore_area (20,8,61,15, buff);
     free((char *)buff);
  }
  if (iq_type == IQ9000)
     IQ_Disconnect (comms.port);
  else
     IQterminate (comms.port, 2);
}

int serial_model ( int model )
{
   return ((model == IQ_7000) || (model == IQ_7200) || (model == IQ_7400) ||
           (model == IQ_7600) || (model == IQ_7100M) || (model == IQ_7300M) ||
           (model == IQ_7420) || (model == IQ_7620) || (model == IQ_7690) ||
           
           (model == IQ_8000) || (model == IQ_8200) || (model == IQ_8400) ||
           (model == IQ_8100M) || (model == IQ_8300M) || (model == IQ_8900) ||
           (model == IQ_8920) || (model == IQ_8500M) || (model == IQ_8600M) ||

           (model == IQ_9000) || (model == IQ_9000MK2) || (model == IQ_9200) ||
           (model == IQ_9200MK2) ||

           (model == ZQ_5200) || (model == ZQ_2250) || (model == ZQ_5300) ||
           (model == ZQ_5300M)
          );
}

void highlight_mode_name (int start, int offset, int state)
{
   if (state)
      SetColourPair (palette.bright_white, palette.blue);
   else
      SetColourPair (palette.yellow, palette.blue);
   WriteString (29,8+offset, data_modes[start+offset].view_name);

}

void show_mode (int start, int i)
{
   SetColourPair (palette.yellow, palette.blue);
   write_blanks (29, 8+i, MAX_MODE_LENGTH+1);
   WriteString (29, 8+i, data_modes[start+i].view_name);
   if (data_modes[start+i].selected)
   {
      SetColourPair (palette.red, palette.blue);
      WriteChar (27, 8+i, (char)16);
      WriteChar (53, 8+i, (char)17);
   }
   else
   {
      WriteChar (27, 8+i, ' ');
      WriteChar (53, 8+i, ' ');
   }
}

void show_mode_list (int first, int last)
{
   int i;

   for (i=first; i <= last; i++)
      show_mode (first, i-first);
}

int get_data( int idx)
{  
  int mode_byte;
  char tmp[255];


  if (config_byte2 & MSK_TRANS_MODE) /* Overwrite */
  {
      if (data_modes[idx].location = CARD_MODE)
         mode_byte = 0x39;
      else
        mode_byte = 0x31;

  }
  else
  {
     if (data_modes[idx].location == CARD_MODE)
        mode_byte = 0x38;
     else
        mode_byte = 0x30;
  }

  write_receive_status ("Send Data Request");
  if (iq_type == IQ9000)
  {
      sprintf (tmp, "%s %s%s", GET_9000_DATA_STUB, 
                               data_modes[idx].location == MAIN_MODE ? 
                               MAIN_MEMORY : CARD_MEMORY, 
                               data_modes[idx].filename
              );
      if (IQ_Send_Command (comms.port, tmp) != IQ_ERROR)
      {
          if (IQ_Get_Blocks (comms.port, data_modes[idx].filename, TRUE) != IQ_ERROR)
            return (IQ_SUCCESS);
          else
             return (IQ_ERROR);
      }
      else
        return (IQ_ERROR);
  }
  else
      return(IQtoPC (iq_type, data_modes[idx].name, data_modes[idx].filename, mode_byte));
}

int receive_mode_data ( int idx)
{
   int pos;
   int carry_on;

   pos = idx > 4 ? 4 : idx;

   carry_on = TRUE;
   SetColourPair (palette.black, palette.green);
   WriteString (23, 7+pos, data_modes[idx].name);
   WriteString (50, 7+pos, "???");

   /*   Do Get Data */

   if (!data_modes[idx].selected)
   {
      SetColourPair (palette.bright_white, palette.green);
      WriteString (50, 7+pos, "Skipped");
   }
   else
   {
      if (file_exists (data_modes[idx].filename) &&
          (config_byte2 & MSK_TRANS_MODE)
         )
      {
         if (overwrite_file ())
         {
             if (do_receive(idx) == IQ_SUCCESS)
             {  
                SetColourPair (palette.yellow, palette.green);
                WriteString (50, 7+pos, "OK ");
             }
             else
             {
                 SetColourPair (palette.red, palette.green);
                 WriteString (50, 7+pos, "ERROR");
                 carry_on = FALSE;
             }
         }
         else
         {
             SetColourPair (palette.bright_white, palette.green);
             WriteString (50, 7+pos, "Skipped");
         }

      }
      else       
      { 
          if (do_receive(idx) == IQ_SUCCESS)
          {  
              SetColourPair (palette.yellow, palette.green);
              WriteString (50, 7+pos, "OK ");
          }
          else
          {
              SetColourPair (palette.red, palette.green);
              WriteString (50, 7+pos, "ERROR");
              carry_on = FALSE;
          }
      }

   }
   push_wait();
   return(carry_on);
}

void get_tagged_modes ()
{
  char *buff;
  int i;
  int carry_on;

  buff = (char *)malloc(PORTSIZE(20, 6, 61, 13));
  if (buff != NULL)
  {
    save_area (20,6,61, 13, buff);
    SetColourPair (palette.black, palette.green);
    draw_window (20, 6, 60, 12);
    carry_on = TRUE;
    while (carry_on && (i < number_of_modes))
    {
       if (i > 4)
          scroll_up_area (21,7,59,11);
       carry_on = receive_mode_data(i);
       if (carry_on)
          i++;
    }
    push_wait();
    if (iq_type == IQ9000)
       IQ_Disconnect (comms.port);
    else
       IQterminate (comms.port, 2);
    restore_area(20,6,61,13,buff);
    free((char *)buff);
  }
}

void break_down_mode_buffer ()
{
   int i;
   char block[INFO_BLOCK_SIZE+1];
   
   i = 0;
   number_of_modes=0;
   while (i < strlen(mode_buffer)-1)
   {
      memcpy (&block[0], &mode_buffer[i], INFO_BLOCK_SIZE);
      memset (&data_modes[number_of_modes], 0, sizeof (struct mode_info));
      memcpy (&data_modes[number_of_modes].app_num[0], &block[0], 4);
      if ((block[5] == 0x30) || (block[5] == 0x31))
         data_modes[number_of_modes].location = MAIN_MODE;
      else
         data_modes[number_of_modes].location = CARD_MODE;
      memcpy (&data_modes[number_of_modes].name[0], &block[8], 11);
      strcpy (data_modes[number_of_modes].view_name, data_modes[number_of_modes].name);
      
      strcpy (data_modes[number_of_modes].heading, data_modes[number_of_modes].name);
      if (data_modes[number_of_modes].location == CARD_MODE)
      {
         strcat (data_modes[number_of_modes].view_name, " (CARD)");
         strcat (data_modes[number_of_modes].heading, " (CARD - ");
      }
      else
      {
         strcat (data_modes[number_of_modes].view_name, " (MAIN)");
         strcat (data_modes[number_of_modes].heading, " (MAIN - ");
      }

      if (config_byte2 & MSK_TRANS_MODE)
         strcat (data_modes[number_of_modes].heading, "REPLACING)");
      else
         strcat (data_modes[number_of_modes].heading, "APPENDING)");

      data_modes[number_of_modes].selected = FALSE;
      
      memcpy (&data_modes[number_of_modes].filename[0], &block[8], 8);
      memcpy (&data_modes[number_of_modes].filename[9], &block[16], 3);
      data_modes[number_of_modes].filename[8] = '.';
      number_of_modes++;
      i+=INFO_BLOCK_SIZE;
   }
}

void create_dynamic_menu ()
{
   char *buff;

   window_size = number_of_modes > 8 ? 9 : number_of_modes+1;

   buff = (char *)malloc(PORTSIZE(25, 6, 56, 6+window_size+6));
   if (buff != NULL)
   {
     save_area (25,6,56,6+window_size+6, buff);
     SetColourPair (palette.yellow, palette.blue);
     draw_window (25,6,55,6+window_size+5);
     window_title (25,55,6,"Organiser Data Modes", palette.red, palette.blue);
     SetColourPair (palette.bright_white, palette.blue);
     show_mode_list(0, number_of_modes > 8 ? 8 : number_of_modes - 1);
     draw_button (30, 6+window_size+3, 4, "&Ok", BUTTON_UP);
     draw_button (43, 6+window_size+3, 8, "&Cancel", BUTTON_UP);
     if (tag_modes())
     {
        if (tagged > 0)
           get_tagged_modes ();
        else
        {
          if (iq_type == IQ9000)
             IQ_Disconnect (comms.port);
          else
             IQterminate (comms.port, 2);
        }
     }
     restore_area (25,6,56,6+window_size+6, buff);
     free((char *)buff);
   }
}

void mode_help()
{
   char *buff;

   buff = (char *)malloc(PORTSIZE(15, 8, 66, 16));
   if (buff != NULL)
   {
      save_area (15,8,66,16,buff);
      SetColourPair (palette.black, palette.green);
      draw_window (18,8,62,15);
      window_title (18,62,8,"Mode Selection Help", palette.red, palette.green);
      SetColourPair (palette.black, palette.green);
      WriteString (21, 10, "F1        - Display this help page.");
      WriteString (21, 11, "CSR-UP/DN - Move through mode list.");
      WriteString (21, 12, "SPACE     - Tag/Untag highlighted mode.");
      centre_push_button (18,62,14, "&Continue");
      restore_area (15,8,66,16,buff);
      free((char *)buff);
   }
}

void update_status_bar ()
{
   SetColourPair (palette.bright_white, palette.bright_white);
   WriteChar (20+block_count, 16, ' ');
}

void clear_status_bar ()
{
   SetColourPair (palette.bright_white, palette.black);
   write_blanks (21, 16, 40);
   block_count = 0;
}

void open_receive_statics (char name[])
{
   recv_buff = (char *)malloc(PORTSIZE(15, 14, 66, 21));   
   if (recv_buff != NULL)
   {
      save_area (15, 14, 66, 21, recv_buff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (15, 14, 65, 19);
      SetColourPair (palette.yellow, palette.blue);
      clear_status_bar();
      SetColourPair (palette.light_red, palette.blue);
      WriteString (25, 18, "Status : ");
      window_title (15, 65, 14, name, palette.light_red, palette.blue);

   }
}

void close_receive_statics ()
{
   if (recv_buff != NULL)
   {
      restore_area (15, 14, 66, 21, recv_buff);
      free ((char *)recv_buff);
   }
}

void write_receive_status ( char status[])
{
   SetColourPair (palette.cyan, palette.blue);
   write_blanks (35, 18, 30);
   WriteString (35, 18, status);
}

int tag_modes ()
{
   int first,
       last;
   int finish;
   int option;
   int result;
   unsigned int key;
   char str[80];

   option = 0;
   finish = FALSE;
   tagged = 0;
   first  = 0;
   last   = ( number_of_modes > 8 ? 8 : number_of_modes - 1);

   while (! finish)
   {
      sprintf (str, "Tagged : %02d", tagged);
      window_title (25, 55, 6+5+window_size, str, palette.bright_white, palette.black);
      highlight_mode_name (first, option, TRUE);
      key = get_key();
      if ((key != K_ESC) && ( key != 'C') && (key != 'c') &&
          (key != 'O') && (key != 'o') && (key !=  K_CR) &&
          (key != K_F1)
         )
         highlight_mode_name ( first, option, FALSE);
      switch (key )
      {
         case K_ESC :
         case 'C'   :
         case 'c'   :
                        simulate_push (43, 6+window_size+3, "&Cancel", 8);
                        push_wait();
                        finish = TRUE;
                        result=FALSE;
                        IQterminate (comms.port, 2);
                        break;
         case K_F1  :   mode_help();
                        break;
         case 'O'   :
         case 'o'   :
         case K_CR  :   simulate_push (30, 6+window_size+3, "&Ok", 4);
                        push_wait();
                        data_modes[first+option].selected = TRUE;
                        tagged++;
                        result=TRUE;
                        finish=TRUE;
                        break;
         case K_CSR_UP : if (option > 0)
                            option--;
                         else
                         {
                            if (first > 0)
                            {
                               first--;
                               last--;
                               option=0;
                               show_mode_list(first,last);
                            }
                         }
                         break;
         case K_CSR_DN : if (first+option < last)
                           option++;
                         else
                         {
                            if (last < number_of_modes-1)
                            {
                               first++;
                               last++;
                               option=8;
                               show_mode_list (first, last);
                            }
                         }
                         break;
         case K_SPACE  : if (data_modes[first+option].selected)
                            tagged--;
                         else
                            tagged++;
                         data_modes[first+option].selected = !data_modes[first+option].selected;
                         show_mode (first, option);
                         break;
         default       : break;
      }
   }
   return(result);
}

int request_directory ()
{
   char *buff;
   int status;
   int result;

   result = FALSE;
   buff = (char *)malloc(PORTSIZE(20, 8, 61, 14));
   if (buff != NULL)
   {
      save_area (20,8,61,14,buff);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (20,8,60,13);
      SetColourPair (palette.yellow, palette.blue);
      centre_text (21,59,10,"Requesting Mode List From Organiser");
      centre_text (21,59,11, "** PLEASE WAIT **");

      status = IQconnect (comms.port, RXBUFSIZE, TXBUFSIZE);
      restore_area (20,8,61,14,buff);
      free((char *)buff);
      if ((status != IQ7000) && (status != IQ8000))
      {
         result = FALSE;
         comms_error (status);
      }
      else
        result = TRUE;
   }
   return(result);
}

int do_receive ( int i)
{
   int state;
   
   open_receive_statics (data_modes[i].heading);
   
    state = get_data( i );
    if (state == IQ_SUCCESS)
    {
       write_receive_status ("Data Transfer Complete");
    }
    else
    {
       write_receive_status ("Possible Comms Error");
       comms_error (state);
    }
    close_receive_statics();
    return(state);
}





