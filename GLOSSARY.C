#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "keys.h"
#include "glossary.h"
#include "palette.h"
#include "windows.h"
#include "iq.h"

extern int srchlen;
extern struct palette_str palette;
extern int highlighted_glossary_index;
extern char srchstr[];

extern int file_exists (char name[]);
extern void centre_text (int x1, int x2, int y, char msg[]);
extern int find_name_in_index ( char str[]);
extern void query_glossary ( void );
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern unsigned int get_key ( void );
extern void push_wait ( void );
extern int draw_button (int x, int y, int width, char msg[], int state);
extern void simulate_push (int x, int y, char msg[], int width);
extern void centre_push_button (int x1, int x2, int y, char msg[]);

void do_glossary ( void );
void show_index ( void);
void process_letter ( unsigned int ch, int state);
int get_letter_column (char ch);
int get_letter_row (char ch);
void glossary_help ( void );
void select_index ( char letter);
void draw_index_segment ( void );
void clear_index_segment ( void );
void init_glossary_index(void);
void glossary_not_found (void);

char *index_seg;
char glossary_index[MAX_INDEX][30];

void init_glossary_index ()
{
    strcpy (glossary_index[0], "Access");
    strcpy (glossary_index[1], "Actuator");
    strcpy (glossary_index[2], "Address");
    strcpy (glossary_index[3], "Address Mark");
    strcpy (glossary_index[4], "Adjustable Interleave");
    strcpy (glossary_index[5], "Ansi");
    strcpy (glossary_index[6], "Application Program");
    strcpy (glossary_index[7], "Areal Density");
    strcpy (glossary_index[8], "Ascii");
    strcpy (glossary_index[9], "Asme");
    strcpy (glossary_index[10], "Asynchronous Data");
    strcpy (glossary_index[11], "At Interface");
    strcpy (glossary_index[12], "Automatic Backup Of Files");
    strcpy (glossary_index[13], "Auxiliary Memory");
    strcpy (glossary_index[14], "Auxiliary Storage Device");
    strcpy (glossary_index[15], "Average Access Time");
    strcpy (glossary_index[16], "Azimuth");
    strcpy (glossary_index[17], "Backup Device");
    strcpy (glossary_index[18], "Backup File");
    strcpy (glossary_index[19], "Baud Rate");
    strcpy (glossary_index[20], "Bcai");
    strcpy (glossary_index[21], "Bdos");
    strcpy (glossary_index[22], "Bidirectional Bus");
    strcpy (glossary_index[23], "Binary");
    strcpy (glossary_index[24], "Bios");
    strcpy (glossary_index[25], "Bit");
    strcpy (glossary_index[26], "Bit Cell Length");
    strcpy (glossary_index[27], "Bit Cell Time");
    strcpy (glossary_index[28], "Bit Density");
    strcpy (glossary_index[29], "Bit Jitter");
    strcpy (glossary_index[30], "Bit Shift");
    strcpy (glossary_index[31], "Block");
    strcpy (glossary_index[32], "Boot");
    strcpy (glossary_index[33], "Buffer");
    strcpy (glossary_index[34], "Buffered Seek");
    strcpy (glossary_index[35], "Bus");
    strcpy (glossary_index[36], "Byte");
    strcpy (glossary_index[37], "Cache Memory");
    strcpy (glossary_index[38], "Capacity");
    strcpy (glossary_index[39], "Carriage Assembly");
    strcpy (glossary_index[40], "Central Processor Unit");
    strcpy (glossary_index[41], "Character");
    strcpy (glossary_index[42], "Chip");
    strcpy (glossary_index[43], "Clock Rate");
    strcpy (glossary_index[44], "Closed Loop");
    strcpy (glossary_index[45], "Cluster Size");
    strcpy (glossary_index[46], "Code");
    strcpy (glossary_index[47], "Coercivity");
    strcpy (glossary_index[48], "Command");
    strcpy (glossary_index[49], "Console");
    strcpy (glossary_index[50], "Controller");
    strcpy (glossary_index[51], "Core");
    strcpy (glossary_index[52], "Cpu");
    strcpy (glossary_index[53], "Crash");
    strcpy (glossary_index[54], "Cyclic Redundancy Check");
    strcpy (glossary_index[55], "Cylinder");
    strcpy (glossary_index[56], "Daisy Chain");
    strcpy (glossary_index[57], "Data");
    strcpy (glossary_index[58], "Data Access");
    strcpy (glossary_index[59], "Data Address");
    strcpy (glossary_index[60], "Data Base");
    strcpy (glossary_index[61], "Database Management System");
    strcpy (glossary_index[62], "Data Encoding");
    strcpy (glossary_index[63], "Data Field");
    strcpy (glossary_index[64], "Data Separator");
    strcpy (glossary_index[65], "Data Track");
    strcpy (glossary_index[66], "Data Transfer Rate");
    strcpy (glossary_index[67], "Decrease The Flying Height");
    strcpy (glossary_index[68], "Dedicated Servo System");
    strcpy (glossary_index[69], "Default");
    strcpy (glossary_index[70], "Density");
    strcpy (glossary_index[71], "Digital");
    strcpy (glossary_index[72], "Digital Magnetic Recording");
    strcpy (glossary_index[73], "Direct Access");
    strcpy (glossary_index[74], "Directory");
    strcpy (glossary_index[75], "Disc File");
    strcpy (glossary_index[76], "Disc Operating System");
    strcpy (glossary_index[77], "Disc Pack");
    strcpy (glossary_index[78], "Disc Platter");
    strcpy (glossary_index[79], "Disc Storage");
    strcpy (glossary_index[80], "Diskette");
    strcpy (glossary_index[81], "Dos");
    strcpy (glossary_index[82], "Drive");
    strcpy (glossary_index[83], "Drive Select");
    strcpy (glossary_index[84], "Drive Type");
    strcpy (glossary_index[85], "Drop In Drop Out");
    strcpy (glossary_index[86], "Drum");
    strcpy (glossary_index[87], "Ecc");
    strcpy (glossary_index[88], "Electro Static Discharge");
    strcpy (glossary_index[89], "Embedded Servo System");
    strcpy (glossary_index[90], "Erase");
    strcpy (glossary_index[91], "Error");
    strcpy (glossary_index[92], "Esdi");
    strcpy (glossary_index[93], "Execute");
    strcpy (glossary_index[94], "Fci");
    strcpy (glossary_index[95], "Feedback");
    strcpy (glossary_index[96], "Fetch");
    strcpy (glossary_index[97], "Fields");
    strcpy (glossary_index[98], "File");
    strcpy (glossary_index[99], "File Allocation Table");
    strcpy (glossary_index[100], "File Name");
    strcpy (glossary_index[101], "Firmware");
    strcpy (glossary_index[102], "Fixed Disc");
    strcpy (glossary_index[103], "Floppy Disc");
    strcpy (glossary_index[104], "Flux Change");
    strcpy (glossary_index[105], "Flux Changes Per Inch");
    strcpy (glossary_index[106], "Fm");
    strcpy (glossary_index[107], "Format");
    strcpy (glossary_index[108], "Formatted Capacity");
    strcpy (glossary_index[109], "Fpi");
    strcpy (glossary_index[110], "Friction");
    strcpy (glossary_index[111], "Full Height Drive");
    strcpy (glossary_index[112], "G");
    strcpy (glossary_index[113], "Gap");
    strcpy (glossary_index[114], "Gap Length");
    strcpy (glossary_index[115], "Gap Width");
    strcpy (glossary_index[116], "Gcr");
    strcpy (glossary_index[117], "Guard Band");
    strcpy (glossary_index[118], "Half High Drive");
    strcpy (glossary_index[119], "Hard Disc Drive");
    strcpy (glossary_index[120], "Hard Error");
    strcpy (glossary_index[121], "Hard Error Map");
    strcpy (glossary_index[122], "Hardware");
    strcpy (glossary_index[123], "Hda");
    strcpy (glossary_index[124], "Head");
    strcpy (glossary_index[125], "Head Crash");
    strcpy (glossary_index[126], "Head Landing And Takeoff");
    strcpy (glossary_index[127], "Head Landing Zone");
    strcpy (glossary_index[128], "Head Positioner");
    strcpy (glossary_index[129], "Head Slap");
    strcpy (glossary_index[130], "Hexidecimal");
    strcpy (glossary_index[131], "Id Field");
    strcpy (glossary_index[132], "Image Backup Mode");
    strcpy (glossary_index[133], "Index");
    strcpy (glossary_index[134], "Index Time");
    strcpy (glossary_index[135], "Input");
    strcpy (glossary_index[136], "Input Output");
    strcpy (glossary_index[137], "Intelligent Peripheral");
    strcpy (glossary_index[138], "Interface");
    strcpy (glossary_index[139], "Interface Standard");
    strcpy (glossary_index[140], "Interleave Factor");
    strcpy (glossary_index[141], "Interleaving");
    strcpy (glossary_index[142], "Interrupt");
    strcpy (glossary_index[143], "I O Processor");
    strcpy (glossary_index[144], "Kilobyte");
    strcpy (glossary_index[145], "Lan");
    strcpy (glossary_index[146], "Landing Zone");
    strcpy (glossary_index[147], "Latency");
    strcpy (glossary_index[148], "Logic");
    strcpy (glossary_index[149], "Lookup");
    strcpy (glossary_index[150], "Low Level Format");
    strcpy (glossary_index[151], "Lun");
    strcpy (glossary_index[152], "Magnetic Media");
    strcpy (glossary_index[153], "Magnetic Recording");
    strcpy (glossary_index[154], "Mainframe Computer");
    strcpy (glossary_index[155], "Main Memory");
    strcpy (glossary_index[156], "Mean Time Before Failure");
    strcpy (glossary_index[157], "Mean Time To Repair");
    strcpy (glossary_index[158], "Media");
    strcpy (glossary_index[159], "Media Defect");
    strcpy (glossary_index[160], "Megabyte");
    strcpy (glossary_index[161], "Memory");
    strcpy (glossary_index[162], "Microcomputer");
    strcpy (glossary_index[163], "Microinch");
    strcpy (glossary_index[164], "Microsecond");
    strcpy (glossary_index[165], "Millisecond");
    strcpy (glossary_index[166], "Minicomputer");
    strcpy (glossary_index[167], "Mini Slider Heads");
    strcpy (glossary_index[168], "Mini Winchester");
    strcpy (glossary_index[169], "Mneumonic");
    strcpy (glossary_index[170], "Modified Freq");
    strcpy (glossary_index[171], "Multiprocessor");
    strcpy (glossary_index[172], "Multitasking");
    strcpy (glossary_index[173], "Multiuser");
    strcpy (glossary_index[174], "Noise");
    strcpy (glossary_index[175], "Nrz");
    strcpy (glossary_index[176], "Off Line");
    strcpy (glossary_index[177], "Open Collector");
    strcpy (glossary_index[178], "Operating System");
    strcpy (glossary_index[179], "Output");
    strcpy (glossary_index[180], "Parity");
    strcpy (glossary_index[181], "Parking");
    strcpy (glossary_index[182], "Partitioning");
    strcpy (glossary_index[183], "Path");
    strcpy (glossary_index[184], "Peripheral Equipment");
    strcpy (glossary_index[185], "Plated Thin Film Discs");
    strcpy (glossary_index[186], "Platter");
    strcpy (glossary_index[187], "Polling");
    strcpy (glossary_index[188], "Precompensation");
    strcpy (glossary_index[189], "Preventive Maintenance");
    strcpy (glossary_index[190], "Printed Circuit Board");
    strcpy (glossary_index[191], "Processing");
    strcpy (glossary_index[192], "Program");
    strcpy (glossary_index[193], "Protocol");
    strcpy (glossary_index[194], "Radial");
    strcpy (glossary_index[195], "Ram Disc");
    strcpy (glossary_index[196], "Random Access Memory");
    strcpy (glossary_index[197], "Read");
    strcpy (glossary_index[198], "Recalibrate");
    strcpy (glossary_index[199], "Record");
    strcpy (glossary_index[200], "Reduced Write Current");
    strcpy (glossary_index[201], "Reduced Write");
    strcpy (glossary_index[202], "Resolution");
    strcpy (glossary_index[203], "Rll");
    strcpy (glossary_index[204], "Rom");
    strcpy (glossary_index[205], "Rotational Speed");
    strcpy (glossary_index[206], "Scsi");
    strcpy (glossary_index[207], "Sector");
    strcpy (glossary_index[208], "Sector Slip");
    strcpy (glossary_index[209], "Seek");
    strcpy (glossary_index[210], "Seek Complete");
    strcpy (glossary_index[211], "Sequential Access");
    strcpy (glossary_index[212], "Servo Track");
    strcpy (glossary_index[213], "Setup");
    strcpy (glossary_index[214], "Silicon");
    strcpy (glossary_index[215], "Skewing");
    strcpy (glossary_index[216], "Smd");
    strcpy (glossary_index[217], "Soft Error");
    strcpy (glossary_index[218], "Software");
    strcpy (glossary_index[219], "Software Patch");
    strcpy (glossary_index[220], "Spindle");
    strcpy (glossary_index[221], "Spindle Motor");
    strcpy (glossary_index[222], "St 506 St 412 Interface");
    strcpy (glossary_index[223], "Step");
    strcpy (glossary_index[224], "Stepper Motor");
    strcpy (glossary_index[225], "Step Pulse");
    strcpy (glossary_index[226], "Step Time");
    strcpy (glossary_index[227], "Storage Capacity");
    strcpy (glossary_index[228], "Storage Density");
    strcpy (glossary_index[229], "Storage Location");
    strcpy (glossary_index[230], "Storage Module Drive");
    strcpy (glossary_index[231], "Synchronous Data");
    strcpy (glossary_index[232], "Tape Drive");
    strcpy (glossary_index[233], "Thin Film Heads");
    strcpy (glossary_index[234], "Tpi");
    strcpy (glossary_index[235], "Track");
    strcpy (glossary_index[236], "Track Access Time");
    strcpy (glossary_index[237], "Track Density");
    strcpy (glossary_index[238], "Track Following Servo");
    strcpy (glossary_index[239], "Track Pitch");
    strcpy (glossary_index[240], "Tracks Per Inch");
    strcpy (glossary_index[241], "Track Width");
    strcpy (glossary_index[242], "Track Zero");
    strcpy (glossary_index[243], "Track Zero Detector");
    strcpy (glossary_index[244], "Tunnel Erase");
    strcpy (glossary_index[245], "Unformatted");
    strcpy (glossary_index[246], "Upgrade Path");
    strcpy (glossary_index[247], "Verification");
    strcpy (glossary_index[248], "Voice Coil Motor");
    strcpy (glossary_index[249], "Volatile");
    strcpy (glossary_index[250], "Wan");
    strcpy (glossary_index[251], "Wedge Servo System");
    strcpy (glossary_index[252], "Winchester Drive");
    strcpy (glossary_index[253], "Word");
    strcpy (glossary_index[254], "Write");
    strcpy (glossary_index[255], "Write Current");
    strcpy (glossary_index[256], "Write Fault");
    strcpy (glossary_index[257], "Xsmd");
    strcpy (glossary_index[258], "Zbr");
}

void draw_index_segment ()
{
   index_seg = (char *)malloc(PORTSIZE(35, 5, 66, 20));
   if (index_seg != NULL)
   {
      save_area (35,5,66,20,index_seg);   
      SetColourPair (palette.black, palette.white);
      draw_window (35, 5, 65, 19);
      window_title (35, 65, 5, "Glossary Index", palette.red, palette.white);
   }
}

void clear_index_segment()
{
   if (index_seg != NULL)
   {
     restore_area(35,5,66,20,index_seg);
     free((char *)index_seg);
   }
}

int get_letter_column ( char ch)
{
   int result;

    if ((ch >= 'A') && (ch <= 'F'))
       result = 24;
    else
    {
      if ((ch >= 'G') && (ch <= 'L'))
        result = 33;
      else
      {
         if ((ch >= 'M') && (ch <= 'R'))
           result = 41;
         else
         {
           if ((ch >= 'S') && (ch <= 'X'))
              result = 49;
           else
              result = 57;

         }
      }
    }
    return(result);
   
}

int get_letter_row ( char ch)
{
    int result;

    result = 7;

    if ((ch >= 'A') && (ch <= 'F'))
       result += (int)(ch-'A');
    else
    {
      if ((ch >= 'G') && (ch <= 'L'))
        result += (int)(ch-'G');
      else
      {
         if ((ch >= 'M') && (ch <= 'R'))
           result += (int)(ch-'M');
         else
         {
           if ((ch >= 'S') && (ch <= 'X'))
              result += (int)(ch - 'S');
           else
              result += (int)(ch - 'Y');

         }
      }
    }
    return(result);
}

void show_index ()
{
   char i;

   SetColourPair (palette.yellow, palette.blue);
   for (i = 'A'; i <= 'Z'; i++)
   {   
      if (i <= 'F')
         WriteChar (24, 7+(i-'A'), i);
      else
      {
         if (i <= 'L')
            WriteChar (33, 7+(i-'G'), i);
         else
         {
            if (i <= 'R')
               WriteChar (41, 7+(i-'M'), i);
            else
            {
               if (i <= 'X')
                  WriteChar (49, 7+(i-'S'), i);
               else
                  WriteChar (57, 7+(i-'Y'), i);
            }
         }
      }
   }
}

void process_letter( unsigned int ch, int state)
{
   int x,y;

   x = get_letter_column((char)ch);            
   y = get_letter_row((char)ch);
   if (state)
      SetColourPair (palette.yellow, palette.red);
   else
      SetColourPair (palette.yellow, palette.blue);
   WriteChar (x-1,y, ' ');
   WriteChar (x+1,y, ' ');
   WriteChar (x, y, (char)ch);
}

void glossary_help ()
{
   char *buff;

   buff = (char *)malloc(PORTSIZE(15, 4, 66, 16));
   if (buff != NULL)
   {
      save_area (15, 4, 66, 16, buff);
      SetColourPair (palette.black, palette.green);
      draw_window (15, 4, 65, 15);
      window_title (15, 65, 4, "Glossary Help", palette.red, palette.green);
      SetColourPair (palette.black, palette.green);
      WriteString (17,  6, "F1       - Display this help page.");
      WriteString (17,  7, "?        - Perform search query on glossary");
      WriteString (17,  8, "           index.");
      WriteString (17,  9, "Csr Keys - Highlight desired letter.");
      WriteString (17, 10, "ENTER    - Select highlighted letter.");
      WriteString (17, 11, "<letter> - Show glossary entries beginning");
      WriteString (17, 12, "           with <letter>.");

      centre_push_button (15, 65, 14, "&Continue");
      restore_area (15, 4, 66, 16, buff);
      free((char *)buff);
   }
}

void select_index ( char letter)
{
   srchstr[0] = letter;
   srchstr[1] = 0;
   srchlen=1;

   query_glossary();
}

void glossary_not_found()
{
   char *buff;

   buff=(char*)malloc(PORTSIZE(20,8,61,14));
   if(buff!=NULL)
   {
      save_area(20,8,61,14,buff);
      SetColourPair(palette.bright_white, palette.red);
      draw_window(20,8,60,13);
      SetColourPair(palette.yellow,palette.red);
      centre_text(21,60,10,"** Warning : GLOSSARY.IQ Is Missing !");
      centre_push_button(20,60,12,"&Continue");
      restore_area(20,8,61,14,buff);
      free((char*)buff);
   }
}


void do_glossary ()
{
   char         *gloss;
   int          finished;
   unsigned int key;
   char         letter;


   gloss = (char *)malloc(PORTSIZE (20, 6, 61, 17));
   if (gloss != NULL)
   {
      init_glossary_index();
      save_area (20, 6, 61, 17, gloss);
      SetColourPair (palette.bright_white, palette.blue);
      draw_window (20, 6, 60, 16);
      window_title (20, 60, 6, "Glossary Of Computing Terms", palette.red, palette.blue);
      if (!file_exists (GLOSSARY_NAME))
         glossary_not_found();
      show_index();
      draw_button (24, 14, 9, "&? Query", BUTTON_UP);
      draw_button (38, 14, 4, "&Ok",BUTTON_UP);
      draw_button (47, 14, 8, "&Cancel", BUTTON_UP);
      finished = FALSE;
      letter = 'A';
      while (! finished)
      {
         process_letter(letter, TRUE);
         key = get_key();
         process_letter(letter, FALSE);
         switch (key)
         {
            case K_F1 : glossary_help();
                        break;
            case K_CSR_UP : if (letter > 'A')
                               letter--;
                            break;
            case K_CSR_DN : if (letter < 'Z')
                               letter++;
                            break;
            case K_CSR_RT : if (letter+6 <= 'Z')
                              letter+=6;
                            break;
            case K_CSR_LT : if (letter-6 >= 'A')
                               letter-=6;
                            break;
            case '?'     : simulate_push (24, 14, "&? Query", 9);
                           srchlen = 0;
                           srchstr[0]=0;
                           query_glossary();
                           break;
            case K_CR    : select_index ( letter );
                           break;
            case 'c' :
            case 'C' :
            case K_ESC : finished = TRUE;
                         simulate_push (47, 14, "&Cancel", 8);
                         push_wait();
                         break;
            case 'O'  :
            case 'o'  :
                        finished = TRUE;
                        simulate_push (38, 14, "&Ok", 4);
                        push_wait();
                        break;
            default   : 
                        if (((key >= 'A') && (key <= 'Z')) ||
                            ((key >= 'a') && (key <= 'z'))
                           )
                           {
                             if ((key >= 'a') && (key <= 'z'))
                                key = key - 32;
                             letter = (char)key;
                           }
                        break;
         }
      }
      restore_area (20, 6, 61, 17, gloss);
      free((char *)gloss);
   }
}
