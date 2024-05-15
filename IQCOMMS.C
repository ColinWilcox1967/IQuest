#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>          
#include <string.h>
#include <malloc.h>
#include "iq.h"
#include "gf.h"
#include "asiports.h"
#include "config.h"
#include "iqtimer.h"
#include "iqcomms.h"

extern void write_receive_status (char msg[]);
extern void update_status_bar ( void );
extern void clear_status_bar ( void );
extern int file_exists (char name[]);

extern unsigned char config_byte2;
extern unsigned long chars_read;
extern int           block_count;

// Program constants 

char *app[] = {  "",
		 "TEL     1  ",     // 1
		 "TEL     2  ",
		 "TEL     3  ",
		 "TEL FILE1  ",
		 "TEL FILE2  ",     // 5
		 "TEL FILE3  ",
		 "TEL FREE1  ",
		 "TEL FREE2  ",
		 "TEL FREE3  ",
		 "MEMO    1  ",     // 10
		 "OUTLINE 1  ",       
		 "SCHEDULE1  ",
		 "ANN     1  ",
		 "ANN     2  ",
		 "PERIOD  1  ",     // 15
		 "D ALARM 1  ",
		 "BUSINESS1  ",       
		 "BUS FREE1  ",
		 "BUS FREE2  ",
		 "USER'S  DIC",     // 20
		 "DO LIST    ",    
		 "TIME       ",
		 "EXPENSE    "      // 23
		 };
		    

unsigned char info_block2[64] = {  // Set up not to check checksum
    "0000\t0000\t01000000\t\r\n"};

int block1_len;                    // Used to store lenght of block1
int com_port;                      // Global COM port

void  IQterminate (int ,int );
int   IQconnect ( int,unsigned,unsigned );
int   PCtoIQ (int, char [], char [], int , char );
int   IQtoPC (int, char [], char [], char );
void  delay ( int );
int   recv_info_blocks (int );
int   check_rx_buf ( int );
long  timerset ( int , timer_info * );
int   timeup ( timer_info * );


//  FUNCTION:
//  void IQconnect (int, unsigned, unsigned)
//
//  DESCRIPTION:
//  This function first sets XON / OFF processing, then it tests to see if
//  either a ZQ / IQ-7000 or an IQ-8000 is connected & on-line, if so it then
//  receives the relevant information blocks.
//
//  RETURNS:
//  IQ_7000        - A ZQ / IQ-7000 is connected and on-line.
//  IQ8000        - An IQ-8000 is connected and on-line.
//  IQ_OFFLINE     - No IQ connected or offline
//  CHECKSUM_ERROR - Invalid checksum
//  INVALID_PORT   - invalid port specified
//

int IQconnect(int comport , unsigned rxbufsize, unsigned txbufsize )
{
    int status;

    com_port = comport;
    status =asiopen(com_port,
		    (unsigned)MODE,
		    rxbufsize,
		    txbufsize,
		    BAUD,P_NONE,STOP_BIT,WORD_LEN,ON,ON);

    if (status != ASSUCCESS)
	      return (status);          // Reason why port failed to open
    asixon(com_port,30,70,XON,XOFF);    // Set XON / OFF processing 
    asiclear(com_port,ASINOUT);         // Clear any data in RX / TX buffers

    delay ( 1 );
    asiputs(com_port,"\003D",-1);       // request directory from IQ-8000

    delay(3);    
    
    if (!isrxempty(com_port))
    {
	status = recv_info_blocks( (int) IQ8000  );
	
	return (( status == IQ_SUCCESS ) ?  IQ8000 : status );
    }
    else
    {
	asiclear(com_port,ASINOUT);
	asiputs(com_port,"\004D",-1);   // request directory from IQ-7000

	delay(2);

	if (!isrxempty(com_port))
	{
	    status = recv_info_blocks((int) IQ7000 );
	    if ( status == IQ_SUCCESS )
		return ( IQ7000 );
	    else
		return ( status );
	}
	else
	   return ( IQ_OFFLINE );       // Nothing connected or offline 
    }
}

//  FUNCTION:
//  recv_info_blocks (int iq_type )
//  int iq_type;  - either IQ7000 or IQ8000
//
//  DESCRIPTION:
//  This function receives application information blocks from both
//  ZQ / IQ-7000 & IQ-8000 organisers.
//
//  RETURNS:
//  IQ_SUCCESS     - if successfuly receives info blocks
//  RCV_TIMEOUT    - Timed out waiting for character from IQ
//  CHECKSUM_ERROR - Invalid checksum value
//

int   recv_info_blocks (int iq_type )
{
    int     ch;
    int      buf_offset;
    char     check[8];
    unsigned checksum,checkcode;

    buf_offset = 1;
    ch=0;
    checksum = 0 ;           

    if (iq_type == IQ8000)
    {
	// Recv information block2 - Level II protocol 
	while (ch != LF)
	{
	    if (check_rx_buf(com_port))
		ch= asigetc(com_port);
	    else
	    {
		return (RCV_TIMEOUT);  // timed out
	    }
	}
    }

    buf_offset = 0;
    ch=0;
    while (ch!= EOB)
    {
	// Recv information block1 - Level II & I protocol 

	if (check_rx_buf(com_port))
	    ch = asigetc(com_port);
	else
	{
	   return (RCV_TIMEOUT);  // timed out
	}
	info_block1[buf_offset++] = (unsigned char)ch;
	checksum+=ch;                            // Totup chechsum 
    }
    if (userdef == 1)
    {
	strcpy ( &info_block1[buf_offset-1], info_blk_user );
	buf_offset += 21;
	// info_block1[buf_offset] = (unsigned char) EOB;

    }

    block1_len = (buf_offset / BLK1_LEN);        // Info block1 length

    buf_offset = 0;
    ch = 0;

    // Recv checksum from IQ 

    while (ch != EOB)
    {
	if (check_rx_buf(com_port))
	   ch = asigetc(com_port);
	else
	{
	   return (RCV_TIMEOUT);  // timed out
	}
	check[buf_offset++]=(unsigned char)ch;
    }

    // Convert 4 byte checksum to ASCII

    for (buf_offset=0 ; buf_offset<4 ; buf_offset++)
    {
	if (check[buf_offset]>'9') check[buf_offset]-=7;
	check[buf_offset]-='0';    
    }
    checkcode = check[0] * 16 + check[1] + check[2] * 4096 + check[3] * 256;

    if (checksum!=checkcode)
	return (CHECKSUM_ERROR);
    else
	return ( IQ_SUCCESS );
}

//  FUNCTION:
//  int check_rx_buf ( int port );
//  int com_port ;   specific COM port to check
//
//  DESCRIPTION:
//  Test if characters in RX buffer, timeout if no characters for 30 secs
//
//  RETURNS:
//  TRUE         is there are characters in the receive buffer.
//  RCV_TIMEOUT if no character received for 30 seconds
//

int check_rx_buf ( int com_port )
{
  timer_info wait_time;

  timerset ( MAX_RECV_WAIT , &wait_time );  //wait_time up to 30 seconds

  while (isrxempty ( com_port )  && (timeup ( &wait_time ) != TIMER_STOPPED ));

  return ((timeup( &wait_time ) == TIMER_STOPPED) ?  RCV_TIMEOUT : TRUE );
}

//  FUNCTION:
//  void IQterminate (port , term_type )
//
//  DESCRIPTION:
//  This function terminates the PC-LINK mode on the ZQ /IQ organisers.
//  You are required to execute this function when terminating the IQtoPC()
//  & PCtoIQ()  either normally or as a result of an error.
//  This function basically conserves the battery life of the IQ.
//  by switching it off.
//
//  RETURNS:
//  None.
//

void IQterminate (int port , int term_type )
{
    // Send terminate sequence to the organiser. 
    switch ( term_type )
    {
	case 0:                       // Leave in PC-LINK mode
	   break;
	case 1:
	      asiputs(port,"\004O",-1);        // Switch off IQ
	   break;
	case 2:
	      asiputc(port,(int) IQ7000);             // Switch off PC-LINK
	      asiputc(port,(int) 'E' );
	   break;
    }
    if (term_type == 0)
    {
       asiquit (port);
    }
    else
    {
       // while (istxempty (port));
       delay ( 1 );
       asiquit (port);
    }
}


//  FUNCTION :
//  IQtoPC ( com_port, iq_type , data_name, pc_filename, card, unsigned count)
//
//  DESCRIPTION:
//  This function receives data sent from an IQ and writes to an IQ
//  format file.
//
//  RETURNS:
//  IQ_SUCCESS       - Data received on PC succesfully
//  IQ_OFFLINE       - Either IQ offline or not connected
//  INVALID_PORT     - Invalid port specified
//  CHECKSUM_ERROR   - checksum error
//  FILE_ERROR       - Error opening file
//  MODE_ERROR  
//  RCV_TIMEOUT      - Timeout occured waiting for character from IQ
//
//  LEVEL II protocol
//  If an error occurs on an IQ-8000 an extra  byte is returned with an
//  error code relating to the reason for the error.
//
//  ERROR CODE       REASON                        
//  41h              I/O device error        
//  42h              Memory over             
//  43h              Buffer over             
//  44h              Data error              
//  46h              Secret mode             
//  48h              Application not found   
//  49h              Data not found          
//  4Ah              Data modified           
//  4Bh              Command not supported   
//  FEh              Low Battery             
//  FFh              ON / BREAK key          
//

int IQtoPC (int iq_type, char data_name[],char pc_filename[], char data_mode )
{
    FILE     *fp;
    int      status ;
    int      buf_offset, r=-1;
    long     buf_offsetl;
    int      checkcode,checksum=0;

    char     iqchar;              // store incomming IQ char here
    int      ch;                  // Character to transmit   
    char     check[32];           // stores checksum
    char     str[50];

    // NOTE : info block2                                                    
    // Setup info block2 now as I am not sure what to put in it !!!          
    // I have set the string up to 'not check checksum' values               
    // In the reference Serial Interface Specification for SHARP new OZ / IQ 
    // the schematic diagram appears to contradict the description for info  
    // block2 as it says that each data record on the IQ has a 6 byte chksum 
    // but info block 2 appears to be sent only once in the schematic diagram
    // not after every record as the description leads me to assume - MJG    
	
    ch = 0;

    // Search through info blocks returned from call to recv_info_blocks   
    // for the information block relating to the selected data name        

    for (buf_offset = 0 ; buf_offset < block1_len ; buf_offset++)
    {
	if (strnicmp(data_name,&info_block1[buf_offset*BLK1_LEN+DATA_NAME_OFF],11)==0)
	{
	    // Found it ! -  'r' now  points to start of selected 
	    // application block name                         
	    r=buf_offset;               
	    buf_offset = block1_len;        // terminate loop 
	}
    }
#if  dummy

    // Set data mode for target Ram  Append / Overwrite 

    info_block1[r*BLK1_LEN+4] = '0';        // Data mode 1st byte 0x30

    if (card)
    {
	// Target expansion card RAM  Data mode 2nd byte 
	// If appending data   - set byte 5 in info block1 to 0x38  
	// If overwriting data - set byte 5 in info block1 to 0x39  

	if (info_block1[r*BLK1_LEN+DATA_MODE_OFF] == EXPAND_RAM_APPEND )
	    data_mode = 'A';
	else 
	    data_mode = 'O';  // Must be set to EXPAND_RAM_OVERWRITE 
    }
    else
    {
	// Target main memory  Data mode 2nd byte 
	// If appending data   - set byte 5 in info block1 to 0x30  
	// If overwriting data - set byte 5 in info block1 to 0x31  

	if (info_block1[r*BLK1_LEN+DATA_MODE_OFF] == MAIN_RAM_APPEND )
	    data_mode = 'A';
	else
	    data_mode = 'O';   /* Must be set to MAIN_RAM_OVERWRITE */
    }
#endif 
    // If the append flag has been set open for write append to
    // existing data else open for write overwite existing data.

    sprintf (str, "Opening PC File : %s", pc_filename);
    write_receive_status (str);

    if (config_byte2 & MSK_TRANS_MODE)
       fp = fopen (pc_filename, "wb");
    else
    {
       if (file_exists (pc_filename))
	  fp = fopen (pc_filename, "a+b");
       else
	  fp = fopen(pc_filename, "wb");
    }

    if (fp == NULL)
      return(FILE_ERROR);

    // Put IQ into send mode

    asiputc(com_port,iq_type);
    asiputc(com_port,'R');

    // If we are connected to an IQ-8000
    // we need to send info block2.     
    // LEVEL II protocol requirements   

    if (iq_type == IQ8000)
    {
	asiputb ( com_port , info_block2 , 21 );
    }

    // Now send info block1 - applicable to 
    // both LEVEL I & II protocol.

    asiputb(com_port ,&info_block1[r*BLK1_LEN],BLK1_LEN);

    if (check_rx_buf(com_port))
    {

	// Wait for result from IQ 
	checkcode = asigetc(com_port);
    }
    else
    {
	fclose (fp);
	return (RCV_TIMEOUT);  // timed out
    }

    if (checkcode == ERROR_END && iq_type == IQ8000)
    {
	if (check_rx_buf(com_port))
	   status = asigetc (  com_port );
	else
	{
	   fclose (fp);
	   return (RCV_TIMEOUT);                 // timed out
	}
	fclose (fp);
	return ( status );                       //  Reason for failure 
    }

    if (checkcode != NORMAL_END_R)
    {
	fclose (fp);
	return (CHECKSUM_ERROR);                 // Level I error return
    }

    // Receive the data from the IQ

    write_receive_status ("Receiving Data");
    
    if (iq_type == IQ8000)
    {
	// LEVEL II protocol - recv info block 2 

	while (ch != LF)
	{
	    if (check_rx_buf(com_port))
	    {
		ch = asigetc ( com_port );
	    }
	    else
	    {
		fclose (fp);
		return (RCV_TIMEOUT);  // timed out
	    }
	}
    }

    // Now receive data from the IQ

    chars_read = 0L;
    block_count = 0;
    buf_offsetl = 0;
    while ( ch != EOB )
    {
	 if (check_rx_buf(com_port))
	     ch = asigetc ( com_port );
	 else
	 {
	     fclose (fp);
	     return (RCV_TIMEOUT);  // timed out
	 }
	 iqchar = (unsigned char) ch ;
	 
	 chars_read++;
	 if ((int)(chars_read % 20L) == 0)
	 {
	     block_count++;
	     if (block_count > 40)
		clear_status_bar();
	     else
		update_status_bar();
	 }

	 // write incomming character straight to file MJG 22/4/91
	 if (buf_offsetl++ >= BLK1_LEN )           // Don't write block1 info
	 {
	     fwrite ( &iqchar ,1 ,1 ,fp);
	 }
	 checksum += ch;                         // Totup checksum 
    }
    fclose ( fp );

    // Receive checksum from IQ

    buf_offset = 0;
    ch = 0;
    while ( ch != EOB)
    {
	if (check_rx_buf(com_port))
	   ch = asigetc ( com_port );
	else
	{
	   return (RCV_TIMEOUT);                 // timed out
	}
	check[buf_offset++] = (unsigned char ) ch;
    }

    // Convert checksum 

    for (buf_offset = 0; buf_offset < 4; buf_offset++)
    {
	 if (check[buf_offset] >'9') check[buf_offset] -= 7;
	    check[buf_offset] -= '0';
    }

    checkcode = check[0] * 16   +
		check[1]        +
		check[2] * 4096 +
		check[3] * 256 ;

    if (checksum != checkcode)
    {
	return (CHECKSUM_ERROR);
    }
    else
    {
       return ( IQ_SUCCESS );                   //  Success on a plate !! 
    }
}


// Function    :  PCtoIQ ( )
// iq_type ;
// char data_name[11];
// char pc_filename[128];
// int card ;
// char data_mode ;
//
//  DESCRIPTION:
//  This function transmits data stored on the PC as an IQ format file to
//  either an ZQ / IQ-7000 or an IQ-8000 organiser.
//
//  RETURNS:
//  IQ_SUCCESS       - Data received on PC succesfully
//  IQ_OFFLINE       - Either IQ offline or not connected
//  INVALID_PORT     - Invalid port specified
//  CHECKSUM_ERROR   - checksum error
//  FILE_ERROR       - Error opening file
//  MODE_ERROR  
//  RCV_TIMEOUT      - Timeout occured waiting for character from IQ
//
//  LEVEL II protocol
//  If an error occurs on an IQ-8000 an extra  byte is returned with an
//  error code relating to the reason for the error.
//
//  ERROR CODE       REASON                        
//  41h              I/O device error        
//  42h              Memory over             
//  43h              Buffer over             
//  44h              Data error              
//  46h              Secret mode             
//  48h              Application not found   
//  49h              Data not found          
//  4Ah              Data modified           
//  4Bh              Command not supported   
//  FEh              Low Battery             
//  FFh              ON / BREAK key          
//

int PCtoIQ(int iq_type,char data_name[],char  pc_filename[],int card,char  data_mode)
{
    FILE     *fp;
    int      status ;
    int      buf_offset, r=-1;
    unsigned checkcode,checksum=0;
    char     iqchar;

    int      ch=0;                 // Character to transmit 
    char     check[32];            // stores checksum

    if ((fp=fopen(pc_filename,"rb")) == NULL)
    {
	return (FILE_ERROR) ;
    }

    // Search through info blocks returned from call to recv_info_blocks 
    // for the information block relating to the selected data name      

    for (buf_offset = 0 ; buf_offset < block1_len ; buf_offset++)
    {
	if (strnicmp(data_name,&info_block1[buf_offset*BLK1_LEN+DATA_NAME_OFF],11)==0)
	{
	    // Found it ! -  'r' now  points to start of selected 
	    // application block name                             
	     
	    r=buf_offset;               
	    buf_offset = block1_len;             // terminate loop 
	}
    }

    if (r>=0)   // We have a valid application name 
    {
	// Set data mode for target Ram  Append / Overwrite        
	
	info_block1[r*BLK1_LEN+4] = '1';         // Data mode 1st byte 0x31 

	if (card)
	{
	    // Target expansion card RAM  Data mode 2nd byte 
	    // If appending data   - set byte 5 in info block1 to 0x38
	    // If overwriting data - set byte 5 in info block1 to 0x39
	    
	    info_block1[r*BLK1_LEN+DATA_MODE_OFF]=
		(data_mode == 'A' || data_mode == 'a') ?
		  (unsigned char ) EXPAND_RAM_APPEND
		: (unsigned char ) EXPAND_RAM_OVERWRITE;
	}
	else
	{
	    
	    // Target main memory  Data mode 2nd byte 
	    // If appending data   - set byte 5 in info block1 to 0x30  
	    // If overwriting data - set byte 5 in info block1 to 0x31  
	     
	    info_block1[r*BLK1_LEN+DATA_MODE_OFF] =
		(data_mode == 'A') ?  (unsigned char ) MAIN_RAM_APPEND
				   :  (unsigned char ) MAIN_RAM_OVERWRITE ;
	}

	 // Put IQ into receive mode
	 
	asiputc(com_port,iq_type);
	asiputc(com_port,'S');              // Instruct IQ to receive data 


	// Send Information block1  
		
	for ( buf_offset = 0 ; buf_offset < BLK1_LEN ; buf_offset++)
	{
	    while (istxfull(com_port));
	    asiputc(com_port,info_block1[r*BLK1_LEN+buf_offset]);

	    // Totup checksum value 
	    checksum += info_block1[r*BLK1_LEN+buf_offset];
	}

	// Next send the data from the file on the PC to the IQ

	buf_offset = 0;
	ch = 0;
	while (!feof ( fp ))
	{
	    fread ( &iqchar ,1 ,1 , fp );
	    ch =  ( int ) iqchar;
	    checksum += ch;                      // Totup checksum value 

	    while (istxfull ( com_port ));
	    asiputc( com_port , ch );            // squirt it to IQ 
	}
	// Close file
	fclose ( fp );

	// Send IQ our checksum   
	buf_offset = 0;
	ch = 0;
	sprintf(check,"%02X%02X\r\n\032",checksum&255,checksum>>8);


	while ( ch != EOB)
	{
	    ch = check[buf_offset++];
	    while (istxfull ( com_port ));
	    asiputc (com_port , ch );
	}

	while (istxempty(com_port));     // Wait for xmit buffer to empty

	// Wait for result from IQ

	if (check_rx_buf(com_port))      // Wait until we have a character 
	   checkcode=asigetc(com_port);
	else
	   return (RCV_TIMEOUT);         // timed out


	// Check if an IQ-8000 is attached, if so receive error 
	// code byte. - LEVEL II only                           

	if (checkcode == ERROR_END && iq_type == IQ8000 )
	{
	   if (check_rx_buf(com_port))  
	      status =  asigetc(com_port) ;
	   else
	      return (RCV_TIMEOUT);      // timed out

	      return ( status );         // Reason for failure Level II
	}

	if (checkcode != NORMAL_END)
	{
	    return (CHECKSUM_ERROR);      // Level I error return
	}
	else
	{
	    return ( IQ_SUCCESS );
	}
    }
    else
    {
	return (MODE_ERROR);
    }
}


/*
*
* timerset
* --------
* PURPOSE: This routine sets the timer up to count a number of
*          seconds.
* USAGE:   timerset ( duration,&timer )  ** duration is in seconds **
*          passes the address of the timer structure
* RETURNS: Nothing 
*/

long timerset (int delay,timer_info *timer)
{
   int secs;
 
   secs = asitime ();
   timer->last_time = secs;
   timer->to_go     = delay;
   return 0 ;
}

/*
* timeup
* ------
* PURPOSE: This routine checks to see if the timer has expired.
*          In doing this it also keeps the timer up to date. If the
*          timer expires, then it is reset and TIMER_STOPPED is returned
*          to the caller, otherwise the timer is left running and the 
*          number of seconds left before expiration is returned to the 
*          caller.
* USAGE:   ret = timeup(&timer);
* RETURNS: to_go   : The no of seconds remaining or TIMER_STOPPED
*                    if the timer has expired.
*/

int timeup (timer_info *timer)
{
   int secs, elapsed;

   secs = asitime ();              /* get seconds from clock            */


   elapsed = secs - timer->last_time;   /* the number of secs gone by        */
   if ( elapsed < 0 )
      elapsed += 60;                    /* correction if secs < 0            */

   timer->to_go = timer->to_go - elapsed;/* decrement timer                  */

   if ( timer->to_go <= 0 )
      timer->to_go = TIMER_STOPPED;

   timer->last_time = secs;              /* reset the seconds now            */

   return (timer->to_go);
}


/*
* FUNCTION NAME: DELAY
* --------------
* PURPOSE: This function delays program execution for a specified number of
*          seconds.
* USAGE:   delay ( delay_time  )
* RETURNS: Nothing.
*/

void delay (int delay_time)
{
   int t ;
   timer_info timer1;
   timerset(delay_time,&timer1) ;
   while ((t = timeup(&timer1)) != TIMER_STOPPED);
}


