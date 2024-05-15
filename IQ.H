#define RM_VERSION              "v1.1"
#define CONFIG_FILENAME         "IQ.CFG"

#define MAX_FILE_LIST    10

#define FILE_ATTR_LIST       ( _A_NORMAL | _A_SUBDIR | _A_ARCH | _A_HIDDEN | _A_SYSTEM )

#define PRINTER_LPT1        0
#define PRINTER_LPT2        1
#define PRINTER_LPT3        2
#define PRINTER_LPT4        3

#define HELP_SWITCH         -1
#define CONFIG_SWITCH       -2
#define DATAFILE_SWITCH     -3
#define UNKNOWN_SWITCH      -999

#define BIOS_KEYBOARD           0x16
#define BIOS_KEYBOARD_GET_FLAGS 2

/* Stored in ROM BIOS Segment zero */

#define LOC_KEYBOARD_FLAGS      0x417

#define TRUE                ( 0 == 0 )
#define FALSE               ( ! TRUE )

#define DEFAULT_PR_FILE    "DEFAULT.PR"

