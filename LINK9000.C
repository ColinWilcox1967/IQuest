#include <stdio.h>
#include <string.h>
#include <dos.h>
#include "gf.h"
#include "asiports.h"
#include "iq.h"
#include "config.h"
#include "palette.h"
#include "windows.h"
#include "trans.h"

#define RETRY 5
#define IQ_TIMER 100000


#define IQ_NUL             0
#define IQ_ENQ             5     
#define IQ_ACK             6     
#define IQ_NAK             21
#define IQ_SYN             22
#define IQ_CAN             24
#define IQ_ESC             27     


extern unsigned char config_byte2;
extern int number_of_modes;
extern char mode_buffer[1024];
extern struct palette_str palette;
extern int block_count;

extern void clear_status_bar(void);
extern void update_status_bar(void);
extern int file_exists (char [] );
extern void SetColourPair (short, short);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern void centre_push_button (int x1, int x2, int y, char msg[]);

char tmp[255];

int get_iq9000_dir ( int port );
void break_down_9000_dir ( int location );
void show_iq9000_comms_error ( int errnum );

//
// IQ-8920/9000 specific control commands 
//
// Most of this code nicked from Howard's Windows Version
//

void IQ_Send_Control ( int port, int x );
int  IQ_Get_Control ( int port );
int  IQ_Send_Command ( int port, char *command );
int  IQ_Get_Blocks ( int port, char *info, int show_bar );
int  IQ_Connect ( int port );
void IQ_Disconnect ( int port );

//
// These two shouldnt need to be called directly
//

void IQ_Send_Control(int port, int x)
{
    asiputc(port,IQ_NUL);        
    asiputc(port,IQ_NUL);        
    asiputc(port,IQ_NUL);        
    asiputc(port,IQ_NUL);        
    asiputc(port,IQ_NUL);        
    asiputc(port,150);        
    asiputc(port,130);        
    asiputc(port,x);
}

int IQ_Get_Control(int port)
{
   int           
                 ret;
   long          i;
   unsigned char buffer[16];
        
   ret = IQ_ERROR;
   i = IQ_TIMER;
   while ((getrxcnt(port)<8) && i) 
       i--;
   if (i)
   {
       asigetb(port,buffer,8);
       ret=(int)buffer[7];
   }
   return (ret);
}

int IQ_Send_Command(int port, char *command)
{
    int      i, 
             length, 
             r, 
             ret;
    unsigned checksum;


    r=RETRY;
    do
    {
        IQ_Send_Control(port, IQ_ENQ);
        ret=IQ_Get_Control(port);
        r--;
    }
    while (ret<0 && r>0);
    if (ret>0)
    {
        length=strlen(command);
        checksum=0;
        for (i=0 ; i<length ; i++)
            checksum+=command[i];
        asiputc(port,IQ_NUL);
        asiputc(port,IQ_NUL);
        asiputc(port,IQ_NUL);
        asiputc(port,IQ_NUL);
        asiputc(port,IQ_NUL);
        asiputc(port,150);
        asiputc(port,129);
        asiputc(port,16);
        asiputc(port,255);
        asiputc(port,255);
        asiputc(port,1);
        asiputc(port,64);
        asiputc(port,254);
        asiputc(port,(length+1)%256);
        asiputc(port,(length+1)/256);
        asiputs(port,command,0);
        asiputc(port,checksum%256);
        asiputc(port,checksum/256);
        ret=IQ_Get_Control(port);
    }
    return (ret);
}

int IQ_Get_Blocks(int port, char *info, int show_blocks)
{
   int           
                 length,
                 ret;
   long          i;
   unsigned int  block;
   unsigned char buffer[512];
   FILE          *fp;     
   
  //
  // in reality dir file name will be fixed as IQ9000.DIR on entry
  //


   fp = fopen ( info, "wb" );
   block=0;
   block_count=0;
   do
   {
       ret=IQ_Get_Control(port);
       if (ret==IQ_ERROR) goto error;
       IQ_Send_Control(port, IQ_SYN);
       i=IQ_TIMER;
       while ((getrxcnt(port)<15) && i) i--;
       if (i==0) goto error;
       asigetb(port,buffer,15);
       block=(unsigned int)buffer[8]+256*(unsigned int)buffer[9];
       
       length=buffer[13]+256*buffer[14];
       i=IQ_TIMER*2;
       while ((getrxcnt(port)<length) && i) 
           i--;
       if (i==0) goto error;
       asigetb(port,buffer,length);
       fwrite (buffer, length, 1, fp);
       i=IQ_TIMER;
       while ((getrxcnt(port)<2) && i) 
           i--;
       if (i==0) goto error;
       asigetb(port,buffer,2);
       IQ_Send_Control(port,IQ_ACK);
       i = IQ_TIMER;
       while (i)
          i--;

       if (show_blocks)
       {
           block_count++;
           if (block_count > 40)
               clear_status_bar();
           else
               update_status_bar();
       }

   }
   while (block < 65535);

error:
   fclose(fp);
   return(ret);
}

int IQ_Connect( int port)
{
    int r,
        ret;
        
    ret=IQ_ERROR;
    asiopen(port,ASINOUT|BINARY|NORMALRX,1024,1024,9600,P_ODD,1,8,ON,OFF);
    r=RETRY;
    do
    {
       IQ_Send_Control(port,IQ_ESC);
       IQ_Send_Control(port,IQ_ENQ);
       ret=IQ_Get_Control(port);
       r--;
    }
    while (ret<0 && r>0);
    return(ret);
}

void IQ_Disconnect(int port)
{
   asiquit(port);
   asireset ( port);
}

// My stuff from here onwards
//


void show_IQ9000_comms_error ( int code )
{
   char *buff;
   char msg[40];

   buff = (char *)malloc(PORTSIZE(20,8,61,14));
   if (buff != NULL)
   {
      save_area(20,8,61,14,buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window (20,8,60,13);
      SetColourPair (palette.yellow, palette.red);
      switch (code)
      {
         case IQ9000_NO_CONNECT : strcpy (msg, "Unable To Connect To Organiser");
                                  break;
         case IQ9000_BAD_RCV    : strcpy (msg, "Bad Reception From Organiser");
                                  break;
         case IQ9000_BAD_SND    : strcpy (msg, "Bad Transmission To Organiser");
                                  break;
         case IQ9000_NO_DIR     : strcpy (msg, "Unable To Retrieve Directory");
                                  break;
         default                : sprintf (msg, "Unknown Status Byte : %d", code);
                                  break;
      }
      centre_text (21, 60, 9, msg);
      centre_push_button (21, 60, 11, "&Ok"); 
      restore_area (20, 8, 61, 14, buff);
      free((char *)buff);
   }
}

void get_mode_buffer (char *info)
{
   FILE *fp;
   int  i;

   memset (mode_buffer, 0, sizeof (mode_buffer));
   fp = fopen (info, "rb");
   if (fp != NULL)
   {
       i=0;
       while (! feof (fp))
          mode_buffer[i++]=(char)fgetc(fp);
       mode_buffer[i]=(char)0x1A;
       fclose(fp);
   }
}

void break_down_9000_dir ( int source_memory )
{
    int  i, 
         j;    
    char buffer[255];
    struct find_t c_file;

    i = 0;
    
    _dos_findfirst (IQ9000_DIR_FILE, _A_NORMAL, &c_file);
    if (c_file.size > 1)
    {
       get_mode_buffer(IQ9000_DIR_FILE);
       while (mode_buffer[i] != (char)0x1A) // end of buffer
       {
          j = 0;
          while (mode_buffer[i+j] != (char)0)
          {
             buffer[j] = mode_buffer[i+j];
             j++;
          }
          buffer[j++] = (char)0;

          if (strcmp (&buffer[strlen(buffer)-4], ".IDX") == 0)
          {
              memset ( &data_modes[number_of_modes].selected, 0, sizeof ( struct mode_info ));
              strcpy (data_modes[number_of_modes].filename, buffer);
              strcpy (data_modes[number_of_modes].view_name, buffer);
              strcpy (data_modes[number_of_modes].name, buffer);
              strcpy (data_modes[number_of_modes].heading, buffer);              
              strcpy (data_modes[number_of_modes].app_num, "0000"); // not actually used !!
              data_modes[number_of_modes].location = source_memory;
              data_modes[number_of_modes].selected = FALSE;
      
              while (strlen(data_modes[number_of_modes].view_name) < 12)
                 strcat (data_modes[number_of_modes].view_name, " ");

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


              number_of_modes++;
          }
          i+=j;
       }
    }
}

int get_iq9000_dir ( int port )
{
   int ret;

   ret = 0;
   number_of_modes = 0;
   
   sprintf (tmp, "%s %s", GET_9000_DIR_STUB, MAIN_MEMORY );
   if (IQ_Send_Command (port, tmp) == IQ_ACK )
   {
       if (IQ_Get_Blocks (port, IQ9000_DIR_FILE, FALSE) != IQ_ERROR )
       {
           break_down_9000_dir ( (int)MAIN_MODE );
           sprintf (tmp, "%s %s", GET_9000_DIR_STUB, CARD_MEMORY );
           if (IQ_Send_Command (port, tmp) != IQ_ERROR)
           {
               if (IQ_Get_Blocks (port, IQ9000_DIR_FILE, FALSE) != IQ_ERROR )
                  break_down_9000_dir ( (int)CARD_MODE );
               else
               {
                  ret=IQ_ERROR;
                  show_iq9000_comms_error ( IQ9000_BAD_RCV );
               }
           }
           else
           {
              ret=IQ_ERROR;
              show_iq9000_comms_error ( IQ9000_BAD_SND );
           }
       }
       else
       {
         ret=IQ_ERROR;
         show_iq9000_comms_error (IQ9000_BAD_RCV );
       }
   }
   else
   {
      ret=IQ_ERROR;
      show_iq9000_comms_error (IQ9000_BAD_SND);
   }

   //
   // remove any trace of DIR file(just to tidy up)
   //

   if (file_exists(IQ9000_DIR_FILE))
      remove (IQ9000_DIR_FILE);
   return(ret);
}






