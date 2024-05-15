#define INFO_BLOCK_SIZE  21
#define MAX_DATA_MODES   60
#define MAX_MODE_LENGTH  20
#define MAX_APPNUM_LENGTH 5
#define MAX_FILENAME_SIZE 12

#define CARD_MODE         0
#define MAIN_MODE         1

#define MAIN_MEMORY       "S1:"
#define CARD_MEMORY       "S2:"
#define GET_9000_DIR_STUB "FILE DIR"
#define GET_9000_DATA_STUB "DATA RECEIVE"

#define IQ9000_DIR_FILE   "IQ9000.DIR"

#define IQ9000_NO_CONNECT  -1
#define IQ9000_BAD_RCV     -2
#define IQ9000_BAD_SND     -3
#define IQ9000_NO_DIR      -4

#define IQ_ERROR -1

struct mode_info
{
   int  selected;                       // Has this one been tagged ?
   char app_num[MAX_APPNUM_LENGTH];     // Received As Part Or Ino Block
   char name[MAX_MODE_LENGTH];          // App name for dislaying on screen
   char view_name[MAX_MODE_LENGTH];    // displayable name 
   char filename[MAX_FILENAME_SIZE+1];  // PC Format FileName
   char heading[40];
   int  location;                       // CARD Or Main memory 
} data_modes[MAX_DATA_MODES];



