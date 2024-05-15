#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "iq.h"
#include "windows.h"
#include "palette.h"
#include "keys.h"
#include "glossary.h"
#include "errlist.h"

#define X_START   11
#define X_END     70
#define Y_START   5

#define MAX_SEARCH    10

extern struct palette_str palette;
extern char *index_seg;
extern char glossary_index[MAX_INDEX][30];

extern int file_exists (char name[]);
extern void glossary_not_found(void);
extern void show_error (int errnum);
extern void WriteString(int x, int y, char msg[]);
extern void write_blanks (int x, int y, int n);
extern unsigned int get_key ( void );
extern void draw_index_segment(void);
extern void clear_index_segment(void);
extern void clear_area(int x1, int y1, int x2, int y2);
extern void window_title (int x1, int x2, int y, char msg[], short fg, short bk);
extern void centre_push_button(int x1, int x2, int y, char msg[]);

char d_buff[1024];
char srchstr[MAX_SEARCH+1];
int  srchlen;
int  smallest_visible_index,
     largest_visible_index,
     highlighted_glossary_index;

int  find_name_in_index(char str[]);
void show_search_string ( void );
void show_index_range ( void );
void query_glossary ( void );
void do_query_help(void);
void show_index_entry(int state);
void show_desc (int s);
int word_delimiter (char ch);
void show_description (int size);

void do_query_help()
{
  char *buff;

  buff=(char *)malloc(PORTSIZE(15, 6, 66, 20));
  if(buff!=NULL)
  {
     save_area(15,6,66,20,buff);
     SetColourPair(palette.black, palette.green);
     draw_window(15,6,65,19);
     window_title(15,65,6, "Glossary Query Help", palette.red, palette.green);
     SetColourPair (palette.black, palette.green);
     WriteString (17,  9, "F1        - Display this help page.");
     WriteString (17,  9, "CSR-UP/DN - Highlight desired definition.");
     WriteString (17, 10, "PG-UP/DN  - Page through index entries.");
     WriteString (17, 11, "BS        - Remove characters from keyword.");
     WriteString (17, 12, "CTRL-BS   - Clear search keyword.");
     WriteString (17, 13, "<letter>  - Add letter to keyword.");
     WriteString (17, 14, "ESC       - Abort search.");
     WriteString (17, 15, "ENTER     - Display description of highlighted");
     WriteString (17, 16, "            entry.");
     centre_push_button (15,65,18, "&Continue");
     restore_area(15,6,66,20,buff);
     free((char *)buff);
  }
}

void show_search_string ()
{
   SetColourPair (palette.yellow, palette.black);
   write_blanks(22,9,MAX_SEARCH);
   WriteString(22,9,srchstr);
}

int find_name_in_index ( char search[])
{
   int          found;
   unsigned int index;

   found= FALSE;
   index=0;
   while(!found && (index < MAX_INDEX))
   {
      if (strnicmp(glossary_index[index], search, strlen(search)) == 0)
        found = TRUE;
      else
        index++;
   }
   return (found ? index : -1);
}

int read_description(int index)
{
   char ch;
   int found_desc;
   int entry;
   int length;
   FILE *g_ptr;
   
   if (file_exists (GLOSSARY_NAME))
   {
      g_ptr=fopen(GLOSSARY_NAME, "rb");

      found_desc = FALSE;
      entry=-1;
      length=0;
      while (!found_desc && ! feof(g_ptr))
      {
         ch=(char)fgetc(g_ptr);
         if (ch==(char)0xFF) /* Separator*/
         {
            entry++;
            found_desc=(entry==index);
            if(!found_desc)
               length=0;            
         }
         else
            d_buff[length++] = (ch ^ 0xff);
      }
      fclose(g_ptr);
      return(length);
   }
   else
   {
      glossary_not_found();
      return(-1);
   }
}

int word_delimiter(char ch)
{
   return (strchr (":. ;,?!", ch) != NULL);
}

void show_desc (int size)
{
   short x, y;
   int   offset;
   int   ch;
   int   word_len;
   int   ampersand,
         dollar;

   x = X_START; 
   y = Y_START;
   offset = 0;
   dollar = FALSE;  /* Bright White on Green */
   ampersand = FALSE; /* Red on green */

   SetColourPair (palette.black, palette.green);
   while (offset < size)
   {
      ch = d_buff[offset++];
      if ((ch == '$') || (ch == '&'))
      {
         switch (ch)
         {
            case '$' : if (dollar)
                          SetColourPair (palette.black, palette.green);
                       else
                          SetColourPair (palette.bright_white, palette.green);
                       dollar =!dollar;
                       break;
            case '&' : if (ampersand)
                          SetColourPair (palette.black, palette.green);
                       else
                          SetColourPair (palette.red, palette.green);
                       ampersand=!ampersand;
                       break; 
         }
      }
      else
      {
          if (word_delimiter((char)ch)) /* Word end - so check next work len to see if it fits */
          {
         
             while ((offset < size) && word_delimiter(d_buff[offset]))
               offset++;

             word_len = 0;
             while ((word_len+offset < size) && !word_delimiter (d_buff[word_len+offset]))
                word_len++;
         
             if (x+word_len >= X_END-1) /* off right hand screen edge ?*/
             {
                ch = d_buff[offset];
                offset++;
                x = X_START;
                y++;
             }
          }

          if (ch == 0x0D)
          {
             x = X_START;
             y++;
          }
          else
          {
             WriteChar (x, y, (char)ch);
             x++;
          }
      
          if (x == X_END)
          {
             x = X_START;
             y++;
          }
       }
   }
}

void show_description (int index)
{
   char *desc;
   int desc_length;

   desc=(char*)malloc(PORTSIZE(10, 4, 71, 16));
   if(desc!=NULL)
   {
     save_area(10,4,71,16,desc);
     SetColourPair(palette.black, palette.green);
     draw_window(10,4,70,15);
     window_title(10,70,4, glossary_index[index], palette.red, palette.green);
     desc_length = read_description(index);
     show_desc(desc_length);
     centre_push_button (10, 70, 14, "&Continue");
     restore_area(10,4,71,16,desc);
     free((char *)desc);
   }
   else
     show_error (ERR_NO_MEMORY);
}

void show_index_sequence (int s, int f)
{
   int idx;

   SetColourPair (palette.black, palette.white);
   clear_area (36, 6, 64, 18);
   for (idx=s; idx <= f; idx++)
      WriteString (37, idx-s+6, glossary_index[idx]);
}

void show_index_range ( void )
{
   unsigned int start,
                finish;
   int          idx;

   idx = find_name_in_index (srchstr);
   if (idx == -1) /* Not Found */
      idx = 0;
   start = idx;
   if (start+12 >= MAX_INDEX)
      finish = MAX_INDEX-1;
   else
     finish = start+12;

   show_index_sequence (start, finish);

   SetColourPair (palette.yellow, palette.black);
   WriteString (37, 6, glossary_index[start]);
   largest_visible_index = finish;
   smallest_visible_index = start;
}

void show_index_entry(int state)
{
   int i;

   i=highlighted_glossary_index-smallest_visible_index;

   if (state)
      SetColourPair(palette.yellow, palette.black);
   else
      SetColourPair (palette.black, palette.white);
   WriteString(37,6+i, glossary_index[highlighted_glossary_index]);
   show_search_string();
}

void query_glossary ()
{
   char *q_buff;
   int  finished,
        aborted;
   unsigned int key;


   q_buff = (char *)malloc(PORTSIZE(20, 8, 35, 11));
   draw_index_segment();
   if ((q_buff != NULL) && (index_seg != NULL))
   {
      save_area (20,8,35,11,q_buff);
      SetColourPair (palette.bright_white, palette.red);
      draw_window(20,8,33,10);
      finished = FALSE;
      aborted = FALSE;
      show_index_range();
      highlighted_glossary_index = smallest_visible_index;
      
      while (! finished)
      {
         show_search_string();
         key = get_key();
         switch (key)
         {
            case K_F1 : do_query_help();
                        break;
            case K_PGUP : 
                          if (smallest_visible_index - 12 >= 0)
                          {
                             smallest_visible_index-=13;
                             largest_visible_index-=13;
                             highlighted_glossary_index=largest_visible_index;
                             show_index_sequence(smallest_visible_index, largest_visible_index);
                             show_index_entry(TRUE);
                             if (strlen(glossary_index[highlighted_glossary_index]) > MAX_SEARCH)
                                strncpy (srchstr, glossary_index[highlighted_glossary_index], MAX_SEARCH);
                              else
                                 strcpy (srchstr, glossary_index[highlighted_glossary_index]);
                              srchlen=strlen(srchstr);
                          }
                          break;
            case K_CTRL_BS : srchstr[0]=0;
                             srchlen=strlen(srchstr);
                             show_index_range();
                             break;
            case K_PGDN   : if (largest_visible_index+12 <= MAX_INDEX-1)
                            {
                               largest_visible_index+=13;
                               smallest_visible_index+=13;
                               highlighted_glossary_index=smallest_visible_index;
                               show_index_sequence(smallest_visible_index, largest_visible_index);
                               show_index_entry(TRUE);
                               if (strlen(glossary_index[highlighted_glossary_index]) > MAX_SEARCH)
                                  strncpy (srchstr, glossary_index[highlighted_glossary_index], MAX_SEARCH);
                               else
                                  strcpy (srchstr, glossary_index[highlighted_glossary_index]);
                               srchlen=strlen(srchstr);

                            }
                            break;
            case K_CSR_UP : show_index_entry(FALSE);
                           if (highlighted_glossary_index > smallest_visible_index)
                                highlighted_glossary_index--;
                           else
                           {
                              if (highlighted_glossary_index > 0)
                              {
                                 smallest_visible_index--;
                                 largest_visible_index--;
                                 highlighted_glossary_index=smallest_visible_index;
                                 show_index_sequence(smallest_visible_index, largest_visible_index);
                              }
                           }
                           
                           if (strlen(glossary_index[highlighted_glossary_index]) > MAX_SEARCH)
                              strncpy (srchstr, glossary_index[highlighted_glossary_index], MAX_SEARCH);
                           else
                              strcpy (srchstr, glossary_index[highlighted_glossary_index]);
                           srchlen=strlen(srchstr);
                           show_index_entry (TRUE);
                           break;
             case K_CSR_DN :show_index_entry(FALSE); 
                           if (highlighted_glossary_index < largest_visible_index)
                              highlighted_glossary_index++;
                           else
                           {
                              if (largest_visible_index + 1 <= MAX_INDEX-1)
                              {
                                 smallest_visible_index++;
                                 largest_visible_index++;
                                 highlighted_glossary_index=largest_visible_index;
                                 show_index_sequence(smallest_visible_index, largest_visible_index);
                              }
                           }
                           if (strlen(glossary_index[highlighted_glossary_index]) > MAX_SEARCH)
                              strncpy (srchstr, glossary_index[highlighted_glossary_index], MAX_SEARCH);
                           else
                              strcpy (srchstr, glossary_index[highlighted_glossary_index]);
                           srchlen=strlen(srchstr);
                           show_index_entry(TRUE);
                           break;
            case K_BS : if(srchlen>0)
                        {
                           srchlen--;
                           srchstr[srchlen]=0;
                           show_index_range();
                        }
                        break;
            case K_CR : 
                        show_description (highlighted_glossary_index);
                        break;
            case K_ESC : finished = TRUE;
                         aborted = TRUE;
            default   : if ((key >= 32) && (key <= 127))
                        {    
                           if (srchlen < MAX_SEARCH)
                           {
                              srchstr[srchlen++] = (char)key;   
                              srchstr[srchlen]=0;
                              highlighted_glossary_index = find_name_in_index (srchstr);
                              if (highlighted_glossary_index==-1)
                                 highlighted_glossary_index=0;
                              show_index_range();
                           }
                        }
                        break;

         }
      }
      restore_area (20,8,35,11,q_buff);
      clear_index_segment();
      free((char *)q_buff);
   }
}

