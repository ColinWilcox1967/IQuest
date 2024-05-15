#include <stdio.h>
#include <stdlib.h>
#include <dos.h>

int array[1024];

int offset[]={78, 9, 710, 1087, 543, 267, 397, 455};
char mask[]="AzD2C";

#define NAME "SECURE.H"

FILE *fptr;

void main (int argc, char **argv)
{
   int           i;
   struct        dostime_t dtime;
   unsigned char seed;
   int           r;
   int           byte;
   int           byte_mask;

   printf ("\nIQuest Serial Number Encryptor v1.0\n");
   printf ("(c) IQuest Software 1995.\n\n");

   _dos_gettime (&dtime);
   seed= dtime.hsecond;
   srand(seed);
   if (argc != 1)
   {
      printf ("Encrypting Password : '%s'\n", argv[1]);
      for (i=0; i <= 1023; i++)
      {
         r=rand();
         array[i] = r;
      }

      
      r=rand();
   }
   for (i=0; i < strlen (argv[1]); i++)
   {
      byte = (int)argv[1][i];
      array[offset[i]]=(int)(byte ^ mask[i%5]);
   }
   fptr=fopen(NAME, "wb");
   fprintf (fptr, "int array[]={");
   for (i=0; i<1023; i++)
   {
         if (((1+i) % 10) == 0)
            fprintf (fptr, "%c%c%d,", 0x0d, 0x0a, (int)(array[i] % 256));
         else
            fprintf (fptr, "%d,", (int)(array[i] % 256));
   
   }
   fprintf (fptr, "%d};", (int)(array[1023] % 256));
   fclose (fptr);
      
   printf ("Stored Password Is :-");
   for (i=0; i<8; i++)
   {
      byte_mask = mask[i%5];
      byte = (int)(array[offset[i]] ^ byte_mask);
      printf ("%c",(char)byte);
   }
}



