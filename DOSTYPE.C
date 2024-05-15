#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <dos.h>
#include "dostype.h"


int dos_param1_type,
    dos_param2_type;

int dos_type ( int func, char dos_spec[] );
void establish_dos_types ( int function, char path1[], char path2[] );


/* 
   Requirements For DOS Support Functions .....

             Param 1                   Param 2
   
   COPY      DOS_FILE                  DOS_FILE + DOS_DIRECTORY
   DELETE    DOS_FILE                  <N/A>
   RENAME    DOS_FILE                  DOS_FILE
   REMOVE    DOS_DIRECTORY             <N/A>
   CREATE    DOS_DIRECTORY             <N/A>
*/

int dos_type ( int func, char dos_spec[] )
{
   int           type;
   struct find_t c_file;
   char          local_str[255];

   if ( access (dos_spec, 0) != 0)
   {
      if ((strchr (dos_spec, '?') != NULL) ||
          (strchr (dos_spec, '*') != NULL)
         )
        type = DOS_WILDCARD;
      else
      {
         strcpy (local_str, dos_spec);
         if ((func != DOS_RENAME) && (func != DOS_COPY))
         {
            if (local_str[strlen(local_str)-1] != '\\')
               strcat (local_str, "\\");
            strcat (local_str, ".");
         }

         if (  ( access (local_str, 0) == 0) ||
             ( (local_str[1] == ':') && (local_str[2] == '\\') &&
               (strlen (local_str) <= 3)
             )
            )
           type = DOS_DIRECTORY;
         else
         {
           if ((func == DOS_CREATE) || (func == DOS_REMOVE))
             type = DOS_DIRECTORY;
           else
           {
              if ((func == DOS_RENAME) || (func == DOS_COPY))
                 type = DOS_FILE;
              else
                 type = DOS_PATH_ERROR;
           }
         }
      }
   }
   else
   {
      if ((strchr (dos_spec, '?') != NULL) ||
          (strchr (dos_spec, '*') != NULL)
         )
        type = DOS_WILDCARD;
      else
      {
         if ( _dos_findfirst (dos_spec, FILE_ATTRIBS, &c_file) == 0 )
         {
            if ((c_file.attrib & 16)==16)
               type = ( c_file.name[0] == '.' ? DOS_PATH_ERROR : DOS_DIRECTORY );      
            else
               type = DOS_FILE;
         }
         else
           type = DOS_DIRECTORY;
      }
   }
   return ( type );
}

void establish_dos_types ( int function, char param1[], char param2[] )
{
    switch ( function )
    {
       case DOS_COPY   : 
       case DOS_RENAME : dos_param1_type = dos_type (function, param1);
                         dos_param2_type = dos_type (function, param2);
                         break;
       case DOS_DELETE :
       case DOS_REMOVE :
       case DOS_CREATE : dos_param1_type = dos_type (function, param1);
                         dos_param2_type = DOS_UNUSED;
                         break;
       default         : dos_param1_type = dos_param2_type = DOS_UNUSED;
                         break;
    }
}

