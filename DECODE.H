/* Include file for password decode process */

#define MAX_STEPS      25

#define END_STAGES     'E'
#define TEXT_MESSAGE   'T'
#define USER_PROMPT    'U'
#define COMPUTATION    'C'
#define CONTINUE_TEXT  'X'
#define SCREEN_SHOT    'S'

/* Put indices for decodable models here */

#define DECODE_IQ7000   0
#define DECODE_IQ8000   1
#define DECODE_EL6320   2
#define DECODE_ZQ5200   3
#define DECODE_ZQ5300M  3
#define DECODE_ZQ5300   3
#define DECODE_ZQ2250   4
#define DECODE_IQ9000   5
#define DECODE_IQ8900   6


struct stages
{
   char  type;
   char prompt_text[80];
   int x;
   int y;
   int width;
   int depth;
   int line_count;
   char line[5][30];
   int row[5];
   int column[5];
};



