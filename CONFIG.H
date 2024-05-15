/* Video Mode Changes */

#define MONO_TO_COLOUR         0
#define COLOUR_TO_MONO         1

#define MONO_MODE              0
#define COLOUR_MODE            1

#define DEFAULT_CONFIG_BYTE1   59
#define DEFAULT_CONFIG_BYTE2   0
#define DEFAULT_CONFIG_BYTE3   0

/* Masks for Config Byte 1 */

#define MSK_MEMORY_WRITE       1             /* 0 = BIOS 
                                                1 = Direct To Memory */
#define MSK_BORDER_TYPE        2             /* 1 = Single Line
                                                0 = Double Line */
#define MSK_MENU_TREE          4             /* 1 = Expand 
                                                0 = Dont */

#define MSK_SHADOW_TYPE        8             /* 1 = Drop Shadow
                                                0 = No Shadow */
#define MSK_WINDOW_TYPE        16            /* 1 = Exploding
                                                0 = Plain */
#define MSK_VIDEO_MODE         96            /* 00 = MONO
                                                01 = Colour
                                                10 = Auto Detect */

#define MSK_FILE_VIEWER        128           /* 1 = Hex Viewer
                                                 0 = ASCII */

/* Config Byte 2 */

#define MSK_DATE_ABBREVIATION    1          /* Date Abbreviation
                                                1 = Yes, 0 = No
                                            */
#define MSK_DATE_FORMAT          14         /* Date Display Format 
                                                D/M/Y = 000
                                                M/D/Y = 001
                                                Y/M/D = 010
                                                Y/D/M = 011
                                                D/Y/M = 100
                                                M/Y/D = 101
                                            */
#define MSK_TIME_FORMAT           16      /* Time Format
                                                 HH:MM:SS = 1
                                                 HH:MM    = 0
                                          */
#define MSK_TIME_FRAME            32      /* 1 = 24 hours, 0 = 12hrs */

#define MSK_DAY_NAME              64      /* 1 = Yes, 0 = NO */

#define MSK_TRANS_MODE            128     /* 1 = Overwrite */
                                         /* 0 = Append    */
#define MSK_SINGLE_LINE_BORDER     2
#define MSK_DOUBLE_LINE_BORDER     0

#define MSK_VIDEO_DETECT           64
#define MSK_VIDEO_MONO             0
#define MSK_VIDEO_COLOUR           32


#define MAX_FILENAME_LENGTH    25




