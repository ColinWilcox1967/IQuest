#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "iq.h"
#include "menu.h"
#include "keys.h"
#include "palette.h"
#include "windows.h"
#include "errlist.h"
#include "decode.h"

struct stages decode_stages[10][MAX_STEPS];
int    step=0;
int    dec_idx;

struct dialogue_palette
{
   short edge_foreground,
         edge_background,
         text_foreground,
         text_background,
         input_foreground,
         input_background;
}; 

char line1[80];
char line2[80];
char pw[80];


struct dialogue_palette dialogue;
char   entry[255];
char   output_string[255];

extern int take_a_look_at_the_password;
extern int model;
extern struct palette_str palette;

extern unsigned int get_key(void);
extern void clear_area (int x1, int y1, int x2,int y2);
extern void scroll_up_area(int x1, int y1, int x2, int y2);
extern void centre_push_button (int x1, int x2, int y, char msg[]);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern int warning (void);
extern void show_error (int errnum);
extern void write_blanks (int x, int y, int n);
extern unsigned int getstr (int x, int y, char str[], char hotkeys[], int len);
extern void window_title(int x1, int x2, int y, char msg[], short fg, short bk);
extern int get_yesno ( int x, int y, short fg, short bk );

int show_iq9000_problem ( void );
void el6320_decode(void);
int decode_index (int model);
void init_decode_sequences (void);
int get_user_dialogue (int x, int y, int n, char msg[]);
void set_dialogue_palette (short c1, short c2, short c3, short c4, short c5, short c6);
void get_password (void);
void decode_user_input (char input_stream[], char output_stream[]);
char decode_byte (char input_byte);
void illegal_decode_step(void);
void show_end_of_decode (void);
int  accept_stage (void);
void write_out_string (int stage_type,int step, int x, int y, char msg[]);
void compute_password(int decode_type);

int decode_index (int model)
{
   switch (model)
   {
      case EL_6320  :
                      return (DECODE_EL6320);
                      break;
      case ZQ_5300M :
                      return (DECODE_ZQ5300M);
                      break;
      case ZQ_5200  :
                      return (DECODE_ZQ5200);
                      break;
      case ZQ_5300  : return (DECODE_ZQ5300);
                      break;
      case ZQ_2250  : return (DECODE_ZQ2250);
                      break;
      case IQ_7000  : 
      case IQ_7100M :
      case IQ_7300M :
      case IQ_7420  :
      case IQ_7620  :
      case IQ_7690  :
      case IQ_7200  :
      case IQ_7400  :
      case IQ_7600  : return(DECODE_IQ7000);
                      break;
      case IQ_8000  :
      case IQ_8100M :
      case IQ_8300M :
      case IQ_8500M :
      case IQ_8600M :
      case IQ_8200  :
      case IQ_8400  :
                      return(DECODE_IQ8000);
                      break;
      case IQ_8900    :
      case IQ_8920    : //         
                        // this may well have to be changed to a unique
                        // decoing sequence when i get the machine from andy
                        //
                        return (DECODE_IQ8900);
                        break;
      case IQ_9000    :
      case IQ_9000MK2 :
      case IQ_9200    :
      case IQ_9200MK2 :
                      return (DECODE_IQ9000);
                      break;
      default       : return(-1);    
                      break;  

   }
}

int to_num(char ch)
{
   return((int)(ch-'0'));
}

void el6320_decode()
{
   /*          HI  LO 
               v   v   */
   char lookup[16][16];
   int i,num;

   memset (lookup, 0, sizeof(lookup));
   memset (pw, 0, sizeof(pw));

   lookup[0][1] = ' ';

   for (i=0; i <=0x0E;i++)
      lookup[4][i] = (char)('A'+i);
   for(i=0; i<=0x0A; i++)
      lookup[5][i]=(char)('P'+i);
   lookup[5][11]='?'; lookup[5][12]='!';
   lookup[4][15] = '['; lookup[5][15] = '\"';
   for(i=0;i <=9; i++)
     lookup[6][i] = (char)('0'+i);
   lookup[6][10]=':'; lookup[6][11]='-'; lookup[6][12]='_'; lookup[6][15]='.';
   lookup[7][0]=(char)16; lookup[7][1]=(char)17; lookup[7][2]='\'';
   lookup[7][3]='~'; lookup[7][4]=(char)0x0D; lookup[7][5] = '$';
   lookup[7][6] = (char)157; lookup[7][7]='œ'; lookup[7][8]=(char)26;
   lookup[7][9]=(char)27; lookup[7][10]='('; lookup[7][11]=')';
   lookup[7][12]=']'; lookup[7][13]='#';lookup[7][14]='*';

    /* Lines 1&2 should by now contain 12 digit numbers */

   num = (10 * to_num(line1[4]))+to_num(line1[5]);
   pw[0] = lookup[num / 16][num % 16];
   num = (10 * to_num(line1[6]))+to_num(line1[7]);
   pw[1] = lookup[num/16][num % 16];
   num = (10 * to_num(line1[8]))+to_num(line1[9]);
   pw[2] = lookup[num/16][num%16];
   num = (10 * to_num(line1[10]))+to_num(line1[11]);
   pw[3] = lookup[num / 16][num % 16];

   num = (10 * to_num(line2[4]))+to_num(line2[5]);
   pw[4] = lookup[num / 16][num % 16];
   num = (10 * to_num(line2[6]))+to_num(line2[7]);
   pw[5] = lookup[num/16][num%16];

}

void init_decode_sequences()
{
    int i;

    /* Sharp ZQ5200 Decoding Steps - Same For ZQ5300M, ZQ5300 */

    decode_stages[DECODE_ZQ5200][0].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][0].prompt_text, "Push the $[RESET]$ key whilsts holding down both the"); 
    decode_stages[DECODE_ZQ5200][1].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][1].prompt_text, "$[ON]$ and $9$ keys. &This will cause a diagnostic menu");
    decode_stages[DECODE_ZQ5200][2].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][2].prompt_text, "&to be displayed.");
    decode_stages[DECODE_ZQ5200][3].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_ZQ5200][3].prompt_text, "Press the $[%c]$ key once to show the next menu.", 31);
    decode_stages[DECODE_ZQ5200][4].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][4].prompt_text, "Choose the &memory dump& option by pressing key $4$.");
    decode_stages[DECODE_ZQ5200][5].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][5].prompt_text, "When prompted for a memory address, enter the value &1FE83&,");
    decode_stages[DECODE_ZQ5200][6].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][6].prompt_text, "and then press $[ENTER]$");
    decode_stages[DECODE_ZQ5200][7].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][7].prompt_text, "Both hexadecimal and character code should now have been");
    decode_stages[DECODE_ZQ5200][8].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][8].prompt_text, "displayed on the organiser's screen.");
    decode_stages[DECODE_ZQ5200][9].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_ZQ5200][9].prompt_text, "The first seven characters, or the characters upto the $'%c'$", 17);
    decode_stages[DECODE_ZQ5200][10].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][10].prompt_text, "constitute the password.");
    decode_stages[DECODE_ZQ5200][11].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][11].prompt_text, "&Push the& $[RESET]$ &button to exit the diagnostic menu.&");
    decode_stages[DECODE_ZQ5200][12].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][12].prompt_text, "To remove the password from your machine, press $[SHIFT$] and");
    decode_stages[DECODE_ZQ5200][13].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][13].prompt_text, "$[SECRET]$ keys together.");

    decode_stages[DECODE_ZQ5200][14].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_ZQ5200][14].prompt_text, "You will be shown a screen similar to that above.");
    decode_stages[DECODE_ZQ5200][14].x = 30;
    decode_stages[DECODE_ZQ5200][14].y = 5;
    decode_stages[DECODE_ZQ5200][14].width=20;
    decode_stages[DECODE_ZQ5200][14].depth=5;
    decode_stages[DECODE_ZQ5200][14].line_count=2;
    decode_stages[DECODE_ZQ5200][14].column[0]=3;
    decode_stages[DECODE_ZQ5200][14].row[0]=2;
    decode_stages[DECODE_ZQ5200][14].column[1]=3;
    decode_stages[DECODE_ZQ5200][14].row[1]=3;
    strcpy(decode_stages[DECODE_ZQ5200][14].line[0], "SECRET OFF");
    strcpy(decode_stages[DECODE_ZQ5200][14].line[1], "[_         ]");

    decode_stages[DECODE_ZQ5200][15].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ5200][15].prompt_text, "$WARNING$ : &Do not select any other option from the diagnostics");
    decode_stages[DECODE_ZQ5200][16].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][16].prompt_text, "          &menu, other than the $Memory Dump$ option.& Selecting");
    decode_stages[DECODE_ZQ5200][17].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ5200][17].prompt_text, "          &any other option will cause loss of data.");

    decode_stages[DECODE_ZQ5200][18].type = END_STAGES;


    /* Sharp ZQ-2250 Decoding Steps */

    decode_stages[DECODE_ZQ2250][0].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][0].prompt_text, "Push the $[RESET]$ switch whilst holding both the");
    decode_stages[DECODE_ZQ2250][1].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ2250][1].prompt_text, "$[ON]$ and $9$ keys down.");
    decode_stages[DECODE_ZQ2250][2].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][2].prompt_text, "&Although not visible on the screen, you have now&");
    decode_stages[DECODE_ZQ2250][3].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ2250][3].prompt_text, "&accessed a hidden diagnostics menu.&");
    decode_stages[DECODE_ZQ2250][4].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][4].prompt_text, "Press the $3$ key to access the &memory dump& function.");
    decode_stages[DECODE_ZQ2250][5].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_ZQ2250][5].prompt_text, "You will see a screen similar to that below.");
    decode_stages[DECODE_ZQ2250][5].line_count=4;
    decode_stages[DECODE_ZQ2250][5].x=26;
    decode_stages[DECODE_ZQ2250][5].y=14;
    decode_stages[DECODE_ZQ2250][5].width=35;
    decode_stages[DECODE_ZQ2250][5].depth=5;
    for (i=0; i<=3; i++)
    {
        decode_stages[DECODE_ZQ2250][5].row[i] = i+1;
        decode_stages[DECODE_ZQ2250][5].column[i]=3;
    }
    
    sprintf (decode_stages[DECODE_ZQ2250][5].line[0], "x x x x x x x %c   (Character)", 17); 
    strcpy ( decode_stages[DECODE_ZQ2250][5].line[1], "20 20 20 20       (Hex Codes)");
    strcpy ( decode_stages[DECODE_ZQ2250][5].line[2], "20 20 20 20");
    strcpy ( decode_stages[DECODE_ZQ2250][5].line[3], "  0 0 0 0         (Address)");
    decode_stages[DECODE_ZQ2250][6].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_ZQ2250][6].prompt_text, "&Repeatedly& press the SEARCH KEY ($[%c]$) until the address", 31);
    decode_stages[DECODE_ZQ2250][7].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ2250][7].prompt_text, "has changed to $'0380'$.");
    decode_stages[DECODE_ZQ2250][8].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][8].prompt_text, "Both hexadecimal and character codes are shown");
    decode_stages[DECODE_ZQ2250][9].type = CONTINUE_TEXT;
    sprintf (decode_stages[DECODE_ZQ2250][9].prompt_text, "on the screen. &Characters shown before the& $%c$ &or the&", 17);
    decode_stages[DECODE_ZQ2250][10].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ2250][10].prompt_text, "&first seven characters from the top are the password.&");
    decode_stages[DECODE_ZQ2250][11].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][11].prompt_text, "&Make a note of this password.");
    decode_stages[DECODE_ZQ2250][12].type=TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][12].prompt_text, "Push the $[RESET]$ button to exit the diagnostic menu.");
    decode_stages[DECODE_ZQ2250][13].type=TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][13].prompt_text, "Turn the organiser on and press the $[SHIFT]$ and $[SECRET]$ keys.");
    decode_stages[DECODE_ZQ2250][14].type=TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][14].prompt_text, "Enter the password to turn the secret mode off.");
    decode_stages[DECODE_ZQ2250][15].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_ZQ2250][15].prompt_text, "$WARNING$ : &Selecting any other option from the diagnostics menu");
    decode_stages[DECODE_ZQ2250][16].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_ZQ2250][16].prompt_text, "          &will cause data in the organiser to be lost !!");


    decode_stages[DECODE_ZQ2250][17].type = END_STAGES;


    /* Sharp EL-6320 Decoding Steps */

    decode_stages[DECODE_EL6320][0].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_EL6320][0].prompt_text, "Power on. (Press the $[ON]$ key).");
    decode_stages[DECODE_EL6320][1].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_EL6320][1].prompt_text, "Press $[CALC]$ (&Select Calculator Mode&)."); 
    decode_stages[DECODE_EL6320][2].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_EL6320][2].prompt_text, "The display should look like that shown below ...");
    decode_stages[DECODE_EL6320][2].x = 30;
    decode_stages[DECODE_EL6320][2].y=15;
    decode_stages[DECODE_EL6320][2].width=20;
    decode_stages[DECODE_EL6320][2].depth=2;
    decode_stages[DECODE_EL6320][2].line_count=1;
    decode_stages[DECODE_EL6320][2].row[0] = 1;
    decode_stages[DECODE_EL6320][2].column[0]= 18;
    strcpy (decode_stages[DECODE_EL6320][2].line[0], "0.");

    decode_stages[DECODE_EL6320][3].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_EL6320][3].prompt_text, "$Press the following keys, to store number below into memory.$");
    decode_stages[DECODE_EL6320][4].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_EL6320][4].prompt_text, "$[CM]$ $[3]$ $[8]$ $[9]$ $[3]$ $[M+]$ (&Leave out all spaces and brackets&).");
    decode_stages[DECODE_EL6320][5].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_EL6320][5].prompt_text, "(The letter '&M&' should now appear on the first line of"); 
    decode_stages[DECODE_EL6320][6].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_EL6320][6].prompt_text, "the display.)");
    
    decode_stages[DECODE_EL6320][7].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_EL6320][7].prompt_text, "Press the $[RESET]$ switch on the back of the organiser whilst");
    decode_stages[DECODE_EL6320][8].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_EL6320][8].prompt_text, "pressing both the $[C.CE]$ and $[OFF]$ keys together.");
    decode_stages[DECODE_EL6320][9].type = SCREEN_SHOT;
    sprintf (decode_stages[DECODE_EL6320][9].prompt_text, "Press the $[%c]$ key. The screen should look that shown above.", 31); 
    decode_stages[DECODE_EL6320][9].x=30; decode_stages[DECODE_EL6320][9].y=5;
    decode_stages[DECODE_EL6320][9].width=20; decode_stages[DECODE_EL6320][9].depth=2;
    decode_stages[DECODE_EL6320][9].line_count=1;
    decode_stages[DECODE_EL6320][9].row[0]=1; decode_stages[DECODE_EL6320][9].column[0]=1;
    strcpy (decode_stages[DECODE_EL6320][9].line[0], "D M P");
    decode_stages[DECODE_EL6320][10].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_EL6320][10].prompt_text, "Press $[ENTER]$.");

    decode_stages[DECODE_EL6320][11].type = USER_PROMPT;
    decode_stages[DECODE_EL6320][11].x=15; decode_stages[DECODE_EL6320][11].y=18;
    decode_stages[DECODE_EL6320][11].width=12;
    decode_stages[DECODE_EL6320][11].line_count=1; /* Entry number from prompting later */
    strcpy(decode_stages[DECODE_EL6320][11].prompt_text, "Enter the 12-digit number shown on screen");

    decode_stages[DECODE_EL6320][12].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_EL6320][12].prompt_text, "Press $[%c]$.", 31); 

    decode_stages[DECODE_EL6320][13].type = USER_PROMPT;
    decode_stages[DECODE_EL6320][13].x=15; decode_stages[DECODE_EL6320][13].y=18;
    decode_stages[DECODE_EL6320][13].width=12;
    decode_stages[DECODE_EL6320][13].line_count=2; /* Entry number from prompting later */
    strcpy(decode_stages[DECODE_EL6320][13].prompt_text, "Enter the 12-digit number shown on screen");

    decode_stages[DECODE_EL6320][14].type = COMPUTATION;
    decode_stages[DECODE_EL6320][15].type = END_STAGES;
    
    
    
    /* Sharp IQ-7XXX Series Decoding Steps */

    decode_stages[DECODE_IQ7000][0].type = TEXT_MESSAGE; 
    strcpy (decode_stages[DECODE_IQ7000][0].prompt_text, "Please ensure that the organiser is switched $off$.");
    decode_stages[DECODE_IQ7000][1].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ7000][1].prompt_text, "Insert the &SECRET CARD& into the card slot.");
    decode_stages[DECODE_IQ7000][2].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ7000][2].prompt_text, "Lock the EPROM card in place.");
    decode_stages[DECODE_IQ7000][3].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ7000][3].prompt_text, "Turn the organiser back on.");
    decode_stages[DECODE_IQ7000][4].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ7000][4].prompt_text, "$You will be shown a sequence of characters on the organiser's$");
    decode_stages[DECODE_IQ7000][5].type = CONTINUE_TEXT;
    strcpy(decode_stages[DECODE_IQ7000][5].prompt_text, "$display.$");
    decode_stages[DECODE_IQ7000][6].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_IQ7000][6].prompt_text, "The characters shown upto (but not including the $'%c'$ is", 17);
    decode_stages[DECODE_IQ7000][7].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ7000][7].prompt_text, "password for this machine). &Please write this down !!&");
    decode_stages[DECODE_IQ7000][8].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ7000][8].prompt_text, "$You will now be able to remove the password by following the$");
    decode_stages[DECODE_IQ7000][9].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ7000][9].prompt_text, "$guidelines given below.$");
    decode_stages[DECODE_IQ7000][10].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ7000][10].prompt_text, "Turn the organiser &off&, then back &on&.");
    decode_stages[DECODE_IQ7000][11].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ7000][11].prompt_text, "Press the $SHIFT$ and $SECRET$ keys together.");
    
    decode_stages[DECODE_IQ7000][12].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_IQ7000][12].prompt_text, "You will be shown a screen similar to that above.");
    decode_stages[DECODE_IQ7000][12].x = 30;
    decode_stages[DECODE_IQ7000][12].y = 5;
    decode_stages[DECODE_IQ7000][12].width=20;
    decode_stages[DECODE_IQ7000][12].depth=5;
    decode_stages[DECODE_IQ7000][12].line_count=2;
    decode_stages[DECODE_IQ7000][12].column[0]=3;
    decode_stages[DECODE_IQ7000][12].row[0]=2;
    decode_stages[DECODE_IQ7000][12].column[1]=3;
    decode_stages[DECODE_IQ7000][12].row[1]=3;
    strcpy(decode_stages[DECODE_IQ7000][12].line[0], "SECRET OFF");
    strcpy(decode_stages[DECODE_IQ7000][12].line[1], "[_         ]");

    decode_stages[DECODE_IQ7000][13].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ7000][13].prompt_text, "Type in the password you noted earlier and hit $ENTER$.");
    
    decode_stages[DECODE_IQ7000][14].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ7000][14].prompt_text, "If all is succesful, the message &SECRET OFF& will be shown,");
    decode_stages[DECODE_IQ7000][15].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ7000][15].prompt_text, "the password is now disabled for this machine.");

    decode_stages[DECODE_IQ7000][16].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ7000][16].prompt_text, "$NOTE$ : &In this state the password is disabled until the machine is&");
    decode_stages[DECODE_IQ7000][17].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ7000][17].prompt_text, "       &switched off. Data can be transfered at any time until the&");

    decode_stages[DECODE_IQ7000][18].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ7000][18].prompt_text, "       &machine is switched off or the password is turned back on&");
    decode_stages[DECODE_IQ7000][19].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ7000][19].prompt_text, "       &by following steps& $[09]$ &onwards above.&");
    
    decode_stages[DECODE_IQ7000][20].type = END_STAGES;


    /* Sharp IQ-8XXX Decoding Steps */

    decode_stages[DECODE_IQ8000][0].type = TEXT_MESSAGE; 
    strcpy (decode_stages[DECODE_IQ8000][0].prompt_text, "Please ensure that the organiser is switched $off$.");
    decode_stages[DECODE_IQ8000][1].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ8000][1].prompt_text, "Insert the &SECRET CARD& into the card slot.");
    decode_stages[DECODE_IQ8000][2].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ8000][2].prompt_text, "Lock the EPROM card in place.");
    decode_stages[DECODE_IQ8000][3].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ8000][3].prompt_text, "Turn the organiser back on.");
    decode_stages[DECODE_IQ8000][4].type = TEXT_MESSAGE;
    strcpy(decode_stages[DECODE_IQ8000][4].prompt_text, "$You will be shown a sequence of characters on the organiser's$");
    decode_stages[DECODE_IQ8000][5].type = CONTINUE_TEXT;
    strcpy(decode_stages[DECODE_IQ8000][5].prompt_text, "$display.$");
    decode_stages[DECODE_IQ8000][6].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_IQ8000][6].prompt_text, "The characters shown upto (but not including the $'%c'$ is", 17);
    decode_stages[DECODE_IQ8000][7].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8000][7].prompt_text, "password for this machine). &Please write this down !!&");
    decode_stages[DECODE_IQ8000][8].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8000][8].prompt_text, "$You will now be able to remove the password by following the$");
    decode_stages[DECODE_IQ8000][9].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8000][9].prompt_text, "$guidelines given below.$");
    decode_stages[DECODE_IQ8000][10].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8000][10].prompt_text, "Turn the organiser &off&, then back &on&.");
    decode_stages[DECODE_IQ8000][11].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8000][11].prompt_text, "Press the $SHIFT$ and $SECRET$ keys together.");
    
    decode_stages[DECODE_IQ8000][12].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_IQ8000][12].prompt_text, "You will be shown a screen similar to that above.");
    decode_stages[DECODE_IQ8000][12].x = 30;
    decode_stages[DECODE_IQ8000][12].y = 5;
    decode_stages[DECODE_IQ8000][12].width=20;
    decode_stages[DECODE_IQ8000][12].depth=5;
    decode_stages[DECODE_IQ8000][12].line_count=2;
    decode_stages[DECODE_IQ8000][12].column[0]=3;
    decode_stages[DECODE_IQ8000][12].row[0]=2;
    decode_stages[DECODE_IQ8000][12].column[1]=3;
    decode_stages[DECODE_IQ8000][12].row[1]=3;
    strcpy(decode_stages[DECODE_IQ8000][12].line[0], "SECRET OFF");
    strcpy(decode_stages[DECODE_IQ8000][12].line[1], "[_         ]");

    decode_stages[DECODE_IQ8000][13].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8000][13].prompt_text, "Type in the password you noted earlier and hit $ENTER$.");
    
    decode_stages[DECODE_IQ8000][14].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8000][14].prompt_text, "If all is succesful, the message &SECRET OFF& will be shown,");
    decode_stages[DECODE_IQ8000][15].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8000][15].prompt_text, "the password is now disabled for this machine.");

    decode_stages[DECODE_IQ8000][16].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8000][16].prompt_text, "$NOTE$ : &In this state the password is disabled until the machine is&");
    decode_stages[DECODE_IQ8000][17].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8000][17].prompt_text, "       &switched off. Data can be transfered at any time until the&");

    decode_stages[DECODE_IQ8000][18].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8000][18].prompt_text, "       &machine is switched off or the password is turned back on&");
    decode_stages[DECODE_IQ8000][19].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8000][19].prompt_text, "       &by following steps& $[09]$ &onwards above.&");
    
    decode_stages[DECODE_IQ8000][20].type = END_STAGES;

    /* Sharp IQ-8900/8920 Decoding Steps */

    decode_stages[DECODE_IQ8900][0].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][0].prompt_text, "Hold down the $ON$ and $D$ keys whilst pressing the reset"); 
    decode_stages[DECODE_IQ8900][1].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][1].prompt_text, "button on the back of the organiser.");   
    decode_stages[DECODE_IQ8900][2].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][2].prompt_text, "The first screen of the &SELF CHECK& menu should be shown.");
    decode_stages[DECODE_IQ8900][3].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][3].prompt_text, "Press $[MENU]$ &three times& to display the third page of options.");
    decode_stages[DECODE_IQ8900][4].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_IQ8900][4].prompt_text, "Use the $[%c]$ and $[%c]$ keys to select the &Memory Dump& option.", 30, 31);
    decode_stages[DECODE_IQ8900][5].type = TEXT_MESSAGE;
    strcpy ( decode_stages[DECODE_IQ8900][5].prompt_text, "Press $[ENTER]$ to select this option.");
    decode_stages[DECODE_IQ8900][6].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_IQ8900][6].prompt_text, "You should now see a memory debug screen, similar to that below.");

    decode_stages[DECODE_IQ8900][6].x = 20;
    decode_stages[DECODE_IQ8900][6].y = 13;
    decode_stages[DECODE_IQ8900][6].width=35;
    decode_stages[DECODE_IQ8900][6].depth=5;
    decode_stages[DECODE_IQ8900][6].line_count=3;
    decode_stages[DECODE_IQ8900][6].column[0]=3;
    decode_stages[DECODE_IQ8900][6].row[0]=1;
    decode_stages[DECODE_IQ8900][6].column[1]=3;
    decode_stages[DECODE_IQ8900][6].row[1]=2;
    decode_stages[DECODE_IQ8900][6].column[2]=3;
    decode_stages[DECODE_IQ8900][6].row[2]=4;

    strcpy(decode_stages[DECODE_IQ8900][6].line[0],   "[P] & [N]       .... 50h");
    strcpy (decode_stages[DECODE_IQ8900][6].line[1],  "SrchUp & SrchDn .... 1000h");
    strcpy (decode_stages[DECODE_IQ8900][6].line[2],  "00000 : xx xx xx xx xx xx");

    decode_stages[DECODE_IQ8900][7].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][7].prompt_text, "The left most column &(00000)& $is the current memory address,");
    decode_stages[DECODE_IQ8900][8].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][8].prompt_text, "and the right most column &(xx xx xx)& $shows the data");
    decode_stages[DECODE_IQ8900][9].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][9].prompt_text, "$contents at this address &IN HEXADECIMAL&.");
    decode_stages[DECODE_IQ8900][10].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][10].prompt_text, "&Each hexadecimal value is displayed using two characters");
    decode_stages[DECODE_IQ8900][11].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][11].prompt_text, "&the corresponding data character for this value can be found");
    decode_stages[DECODE_IQ8900][12].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][12].prompt_text, "&by examining the $HEXADECIMAL CONVERSION TABLE$ supplied with");
    decode_stages[DECODE_IQ8900][13].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][13].prompt_text, "this product. $Write down any useful information.$");



    decode_stages[DECODE_IQ8900][14].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][14].prompt_text, "Use the keys shown at the top of the display to move through");
    decode_stages[DECODE_IQ8900][15].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][15].prompt_text, "memory until one of the addresses given below is shown.");
    decode_stages[DECODE_IQ8900][16].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][16].prompt_text, "256K Model - Start Address is &80000&.");
    decode_stages[DECODE_IQ8900][17].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][17].prompt_text, "512K Model - Start Address is &40000&.");
    decode_stages[DECODE_IQ8900][18].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][18].prompt_text, "right most column. $Useful information should be written down.$");
    decode_stages[DECODE_IQ8900][19].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][19].prompt_text, "Continue the above process until the end of memory is reached.");
    decode_stages[DECODE_IQ8900][20].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][20].prompt_text, "Memory finishes, on both models, at address &BFFFF&.");
    decode_stages[DECODE_IQ8900][21].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ8900][21].prompt_text, "$NOTE : To reset the organiser for normal use, press the reset");
    decode_stages[DECODE_IQ8900][22].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ8900][22].prompt_text, "$button on the back of the machine.");
    decode_stages[DECODE_IQ8900][23].type = END_STAGES;

    /* Sharp IQ-9XXX Decoding Steps */

    decode_stages[DECODE_IQ9000][0].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][0].prompt_text, "Hold down the $ON$ and $D$ keys whilst pressing the reset"); 
    decode_stages[DECODE_IQ9000][1].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][1].prompt_text, "button on the back of the organiser.");   
    decode_stages[DECODE_IQ9000][2].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][2].prompt_text, "The first screen of the &SELF CHECK& menu should be shown.");
    decode_stages[DECODE_IQ9000][3].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][3].prompt_text, "Press $[MENU]$ &twice& to display the third page of options.");
    decode_stages[DECODE_IQ9000][4].type = TEXT_MESSAGE;
    sprintf (decode_stages[DECODE_IQ9000][4].prompt_text, "Use the $[%c]$ and $[%c]$ keys to select the &Memory Dump& option.", 30, 31);
    decode_stages[DECODE_IQ9000][5].type = TEXT_MESSAGE;
    strcpy ( decode_stages[DECODE_IQ9000][5].prompt_text, "Press $[ENTER]$ to select this option.");
    decode_stages[DECODE_IQ9000][6].type = SCREEN_SHOT;
    strcpy (decode_stages[DECODE_IQ9000][6].prompt_text, "You should now see a memory debug screen, similar to that below.");

    decode_stages[DECODE_IQ9000][6].x = 15;
    decode_stages[DECODE_IQ9000][6].y = 12;
    decode_stages[DECODE_IQ9000][6].width=50;
    decode_stages[DECODE_IQ9000][6].depth=6;
    decode_stages[DECODE_IQ9000][6].line_count=5;
    decode_stages[DECODE_IQ9000][6].column[0]=3;
    decode_stages[DECODE_IQ9000][6].row[0]=1;
    decode_stages[DECODE_IQ9000][6].column[1]=3;
    decode_stages[DECODE_IQ9000][6].row[1]=2;
    decode_stages[DECODE_IQ9000][6].column[2]=3;
    decode_stages[DECODE_IQ9000][6].row[2]=3;
    decode_stages[DECODE_IQ9000][6].column[3]=3;
    decode_stages[DECODE_IQ9000][6].row[3]=5;
    decode_stages[DECODE_IQ9000][6].column[4] = decode_stages[DECODE_IQ9000][6].column[3]+26;
    decode_stages[DECODE_IQ9000][6].row[4]=5;

    strcpy(decode_stages[DECODE_IQ9000][6].line[0],   "[P] & [N]       .... 50h");
    sprintf (decode_stages[DECODE_IQ9000][6].line[1], "[%c] & [%c]       .... 100h", 30, 31);  
    strcpy (decode_stages[DECODE_IQ9000][6].line[2],  "SrchUp & SrchDn .... 1000h");
    strcpy (decode_stages[DECODE_IQ9000][6].line[3],  "00000 : xx xx xx xx xx xx");
    strcpy (decode_stages[DECODE_IQ9000][6].line[4], "xx xx : ABCDEFGH");

    decode_stages[DECODE_IQ9000][7].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][7].prompt_text, "The left most column &(00000)& $is the current memory address,");
    decode_stages[DECODE_IQ9000][8].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][8].prompt_text, "and the right most column &(ABCDEFGH)& $shows the data");
    decode_stages[DECODE_IQ9000][9].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][9].prompt_text, "$contents at this address.");

    decode_stages[DECODE_IQ9000][10].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][10].prompt_text, "Use the keys shown at the top of the display to move through");
    decode_stages[DECODE_IQ9000][11].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][11].prompt_text, "memory until one of the addresses given below is shown.");
    decode_stages[DECODE_IQ9000][12].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][12].prompt_text, "256K Model - Start Address is &80000&.");
    decode_stages[DECODE_IQ9000][13].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][13].prompt_text, "512K Model - Start Address is &40000&.");
    decode_stages[DECODE_IQ9000][14].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][14].prompt_text,"The contents of the organiser's memory will be shown in the");
    decode_stages[DECODE_IQ9000][15].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][15].prompt_text, "right most column. $Useful information should be written down.$");
    decode_stages[DECODE_IQ9000][16].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][16].prompt_text, "Continue the above process until the end of memory is reached.");
    decode_stages[DECODE_IQ9000][17].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][17].prompt_text, "Memory finishes, on both models, at address &BFFFF&.");
    decode_stages[DECODE_IQ9000][18].type = TEXT_MESSAGE;
    strcpy (decode_stages[DECODE_IQ9000][18].prompt_text, "$NOTE : To reset the organiser for normal use, press the reset");
    decode_stages[DECODE_IQ9000][19].type = CONTINUE_TEXT;
    strcpy (decode_stages[DECODE_IQ9000][19].prompt_text, "$button on the back of the machine.");
    decode_stages[DECODE_IQ9000][20].type = END_STAGES;
    
    
    
}

void write_out_string (int stage_type, int step, int x, int y, char msg[])
{
   int r, c;
   int idx;
   int amp_state,
       dollar_state;
   char str[80];


   
   idx=0;
   amp_state=dollar_state=FALSE;

   if (stage_type != CONTINUE_TEXT)
   {
       sprintf (str, "[ %02d ]", step+1);
       SetColourPair (palette.yellow, palette.cyan);
       WriteString (x, y, str);
   }
   r=y;
   c=x+7;
   SetColourPair (palette.black, palette.cyan);
   while (idx < strlen(msg))
   {
      if (msg[idx] == '&')
      {  
         if (amp_state)
            SetColourPair(palette.black, palette.cyan);
         else
            SetColourPair (palette.bright_white, palette.cyan);
         amp_state = !amp_state;
      }
      else
      {
         if (msg[idx] == '$')
         {
            if (dollar_state)
               SetColourPair (palette.black, palette.cyan);
            else
               SetColourPair (palette.red, palette.cyan);
            dollar_state = !dollar_state;
         }
         else
         {
            WriteChar (c,r, msg[idx]);
            c++;
         }
      }
      idx++;
   }
}

void set_dialogue_palette (short c1, short c2, short c3, short c4, short c5, short c6)
{
   dialogue.edge_foreground  = c1;
   dialogue.edge_background  = c2;
   dialogue.text_foreground  = c3;
   dialogue.text_background  = c4;
   dialogue.input_foreground = c5;
   dialogue.input_background = c6;
}

void show_end_of_decode()
{
   char *buff;

   buff = (char *)malloc(PORTSIZE(25,11,56, 16));
   if(buff != NULL)
   {
       save_area(25,11,56,16,buff);
       SetColourPair (palette.bright_white, palette.blue);
       draw_window (25, 11, 55, 15);
       SetColourPair(palette.yellow, palette.blue);
       centre_text (25, 55, 12, "Decoding Complete");
       centre_push_button (25, 53, 14, "&Ok");
       restore_area(25, 11, 56, 16, buff);
       free((char *)buff);
   }
}

void compute_password (int decode_type)
{
   char *darra;
   char str[80];

   darra = (char *)malloc(PORTSIZE(20, 7, 61, 14));
   if (darra != NULL)
   {
      save_area(20,7,61,14,darra);
      SetColourPair(palette.bright_white, palette.red);
      draw_window(20,7,60,13);
      window_title (20,60,7,"Organiser Password", palette.yellow, palette.red);
      switch (decode_type)
      {
         case DECODE_EL6320 : el6320_decode();
                              break;
         case DECODE_ZQ2250 : strcpy (pw, "Shown At The Top Of The Screen");
                              break;
         default            : break;
      }
      SetColourPair (palette.bright_white, palette.red);
      
      sprintf (str, "'%s'", pw);
      centre_text (20, 60, 9, "The Password For This Organiser Is");
      SetColourPair (palette.cyan, palette.red);
      centre_text (20, 60, 10, str);
      centre_push_button (20, 60, 12, "&Continue");

      restore_area(20,7,61,14,darra);
      free((char *)darra);
   }
   else
      show_error (ERR_NO_MEMORY);
}

int get_user_dialogue (int x, int y, int max_chars, char msg[])
{
   char *buff;
   char hotkeys[2];
   int status,
       result;

   result = 0;
   buff = (char *)malloc(PORTSIZE(x,y,x+strlen(msg)+5, y+4));
   if (buff!=NULL)
   {
       save_area(x,y,x+strlen(msg)+5,y+4,buff);
       SetColourPair (dialogue.edge_foreground, dialogue.edge_background);
       draw_window (x,y,x+strlen(msg)+4,y+3);
       SetColourPair (dialogue.text_foreground, dialogue.edge_background);
       centre_text (x, x+strlen(msg)+4,y+1, msg);
       SetColourPair (dialogue.input_foreground, dialogue.input_background);
       write_blanks (x+((strlen(msg)+4)/2)-5, y+2, max_chars);
       hotkeys[0] = (char)K_ESC;
       hotkeys[1] = (char)0;
       status = getstr (x+((strlen(msg)+4)/2)-5,y+2, entry, hotkeys, max_chars);
       restore_area (x,y,x+strlen(msg)+5,y+4,buff);
       free((char *)buff);
       result = status;
   }
   else
     show_error (ERR_NO_MEMORY);
   return (result);
}

int accept_stage ()
{
   unsigned int k;
   char *buff;

   buff = (char *)malloc(PORTSIZE(20, 20, 61, 23));
   if(buff!=NULL)
   {
     save_area(20,20,61,23,buff);
     SetColourPair (palette.yellow, palette.blue);
     draw_box(20,20,60,22);
     SetColourPair(palette.bright_white, palette.blue);
     centre_text (20,60,21, "Press Any Key - ESC To Terminate");
   }
   k = get_key();
   if (buff!=NULL)
   {
      restore_area(20,20,61,23,buff);
      free((char *)buff);
   }
   return((int)(k != K_ESC));
}

int show_sample_screen (int x1, int y1, int x2, int y2, int idx, int step)
{
   char *sample;
   int i;


   sample= (char *)malloc(PORTSIZE(x1,y1,x2+1,y2+1));
   if(sample!= NULL)
   {
      save_area(x1,y1,x2+1,y2+1,sample);
      SetColourPair (dialogue.edge_foreground, dialogue.edge_background);
      draw_window(x1,y1,x2,y2);
      SetColourPair (dialogue.text_foreground, dialogue.text_background);
      for(i = 0; i < decode_stages[idx][step].line_count; i++)
         WriteString(x1+decode_stages[idx][step].column[i], y1+decode_stages[idx][step].row[i], decode_stages[idx][step].line[i]);
      get_key();
      restore_area(x1,y1,x2+1,y2+1,sample);
      free((char *)sample);
      return(FALSE);
   }
   else
   {
     show_error (ERR_NO_MEMORY);
     return(TRUE);
   }
}

int check_entry_validity (int decode_type)
{
   int result;

   result= TRUE;
   switch (decode_type)
   {
      case DECODE_EL6320 : if ((strlen(line1) != 12) || 
                               (strlen(line2) != 12) ||
                               (strnicmp(line1, "3893", 4) != 0) ||
                               (strnicmp(line2, "3897", 4) != 0)
                              )
                             result=FALSE;
                           break;
      case DECODE_ZQ2250 : break;
      default            : return (FALSE);
   }
   return (result);
}

int show_iq9000_problem()
{
   int state;
   char *buf;

   state = FALSE;
   buf = (char *)malloc(PORTSIZE(10, 5, 71, 18));
   if (buf != NULL)
   {
      save_area (10,5,71,18,buf);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (10,5,70,17);
      window_title ( 10, 70, 5, "IQ-8920/9x00 Series Password Removal", palette.red, palette.blue);
      SetColourPair (palette.yellow, palette.blue);
      WriteString (12, 7, "Due to an inconsistency in this organiser's method");
      WriteString (12, 8, "of password protection, it is not guaranteed that");
      WriteString (12, 9, "the password can always be removed.");
      WriteString (12, 11, "Therefore, although protected memory can still be viewed");
      WriteString (12, 12, "it will be necessary to manually write down any useful");
      WriteString (12, 13, "information that can be obtained by using this software.");
      SetColourPair (palette.bright_white, palette.blue);
      WriteString (19, 15, "Do you wish to continue with this process ?");
      state = ! get_yesno (32, 16, palette.bright_white, palette.blue);
      restore_area (10, 5, 71, 18, buf );
      free ((char *)buf);
   }
   else
     state = TRUE;

   return (state);
}

void get_password ()
{
    int abort,
        index,
        row,
        step;
    int result;


    if (!warning ())
       return;

    set_dialogue_palette ( palette.bright_white, palette.blue,
                           palette.yellow, palette.blue,
                           palette.bright_white, palette.black
                         );
    step = 0;
    index = 0;
    abort = FALSE;
    row = 5;
    init_decode_sequences();
    dec_idx = decode_index (model);

    
    if ((dec_idx == DECODE_IQ9000) || (dec_idx == DECODE_IQ8900))
        abort = show_iq9000_problem();

    while (!abort && 
           (dec_idx != -1) && 
           (decode_stages[dec_idx][step].type != END_STAGES))
    {


       switch ( decode_stages[dec_idx][step].type)
       {
          case SCREEN_SHOT      : 
                                  write_out_string (decode_stages[dec_idx][step].type,
                                                    index, 4, row, 
                                                    decode_stages[dec_idx][step].prompt_text
                                                   );
                                  if (row < 19)
                                      row++;
                                  else
                                    scroll_up_area(4,5,76,19);
                                  
                                  abort = show_sample_screen(decode_stages[dec_idx][step].x, decode_stages[dec_idx][step].y,
                                                             decode_stages[dec_idx][step].x+decode_stages[dec_idx][step].width,   
                                                             decode_stages[dec_idx][step].y+decode_stages[dec_idx][step].depth,
                                                             dec_idx, step
                                                            );
                                  break;
          case COMPUTATION      : 
                                  /* user entry stored in entry[] */
                                  /* depending on type determines how we use this info */
                                  
                                  if (check_entry_validity (dec_idx))
                                      compute_password(dec_idx);
                                  else
                                  {
                                     show_error (ERR_UNEXPECTED_ENTRY);
                                     abort = TRUE;
                                  }
                                  break;
          case CONTINUE_TEXT    :
          case TEXT_MESSAGE     : 
                                  write_out_string (decode_stages[dec_idx][step].type,
                                                    index, 4, row, 
                                                    decode_stages[dec_idx][step].prompt_text
                                                   );
                                  if (row < 19)
                                      row++;
                                  else
                                    scroll_up_area(4,5,76,19);
                                  break;
          case USER_PROMPT      : result = get_user_dialogue ( decode_stages[dec_idx][step].x,
                                                               decode_stages[dec_idx][step].y,
                                                               decode_stages[dec_idx][step].width,
                                                               decode_stages[dec_idx][step].prompt_text
                                                             );
                                  if (result == 0) /* All Ok for entry */
                                  {
                                     if (decode_stages[dec_idx][step].line_count == 1)
                                        strcpy (line1, entry);
                                     else
                                        strcpy (line2, entry);
                                  }
                                  else
                                  {
                                     show_error (ERR_USER_TERMINATED);
                                     abort = TRUE;
                                  }
                                  break;
          default               :
                                  show_error (ERR_ILLEGAL_STEP);
                                  abort = TRUE;
                                  break;

       }

       step++;
       
       if (decode_stages[dec_idx][step-1].type == SCREEN_SHOT)
       {
          index++;
          abort = FALSE;
       }
       else
       {
           if (! abort)
           {
              /* Full multi line prompt display them all */
              if (decode_stages[dec_idx][step].type != CONTINUE_TEXT)
              {
                 abort = !accept_stage();
                 index++;
                 if (abort)
                    show_error (ERR_USER_TERMINATED);
              }
           }
       }
    }

    if (!abort)
       show_end_of_decode();
    take_a_look_at_the_password = FALSE;
    SetColourPair(palette.black, palette.cyan);
    clear_area(4,5,76,19);
}


