#define MAX_MENU_LEVELS   5
#define MAX_MENUS         40
#define MAX_MENU_OPTIONS  10
#define MAX_OPTION_SIZE   30

#define MENU_NORMAL       0
#define MENU_SEPARATOR   -1
#define MENU_HAS_SUBS    -2
#define MENU_GREYED      -3

#define SUBDIR_MARKER    16


#define FILE_MENU         0
#define CONFIGURE_MENU    1
#define MODEL_MENU        2
#define PROCESS_MENU      3
#define EXIT_MENU         4
#define HELP_MENU         5

#define VIDCONF           6
#define PRCONF            7
#define COMMCONF          8
#define COLCONF           9
#define VIEWCONF          10
#define SHARP_MENU        11
#define SHARP_IQ_MENU     12
#define SHARP_ZQ_MENU     13
#define SHARP_EL_MENU     14
#define APPEARANCE        15
#define PRINTER           16
#define PRINT_CONFIG      17
#define DOS_COMMANDS      18
#define SERIAL_PORT       19
#define SELECT_PORT       20
#define SHARP_XE_MENU     21

#define EL6XXX_MENU       22
#define IQ7XXX_MENU       23
#define IQ8XXX_MENU       24
#define IQ9XXX_MENU       25
#define ZQ1XXX_MENU       26
#define ZQ2XXX_MENU       27
#define ZQ3XXX_MENU       28
#define ZQ4XXX_MENU       29
#define ZQ5XXX_MENU       30
#define ZQ6XXX_MENU       31


/* These labels define leaf menu actions */

#define DO_COMMS          -100
#define ABOUT_IQUEST      -101
#define LEAVE_IQUEST      -102
#define IQ_7000           -106
#define IQ_7200           -107
#define IQ_7400           -108
#define IQ_7600           -109
#define IQ_8000           -110
#define IQ_8200           -111
#define IQ_8400           -112
#define EL_6320           -113
#define ZQ_2250           -114
#define ZQ_5200           -115
#define ZQ_5300           -116
#define VIDEO_MODE        -117
#define BORDER_TYPE       -118
#define SHADOW_TYPE       -119
#define WINDOW_TYPE       -120
#define SCREEN_UPDATE_METHOD -121
#define FILE_VIEWER       -122
#define VIEW_FILE         -123
#define PRINT_FILE        -124
#define CONFIG_SUMMARY    -125
#define TIME_FORMAT       -126
#define DATE_FORMAT       -128
#define PRINTER_LOAD      -129
#define PRINTER_SAVE      -130
#define PRINTER_EDIT      -131
#define DOS_DEL           -132
#define DOS_COPY          -133
#define DOS_REN           -134
#define DOS_RD            -135      
#define DOS_MD            -136
#define GET_PASSWORD      -137
#define GET_DATA          -138
#define SELECT_COM1       -139
#define SELECT_COM2       -140
#define SELECT_COM3       -141
#define SELECT_COM4       -142
#define TRANS_MODE        -143
#define EL_6190           -144
#define ZQ_2700           -145
#define EL_6000           -184
#define EL_6170           -146
#define EL_6330           -147
#define EL_6360           -148
#define EL_6390           -149
#define IQ_7100M          -150
#define IQ_7300M          -151
#define IQ_7420           -152
#define IQ_7620           -153
#define IQ_7690           -154
#define IQ_8100M          -155
#define IQ_8300M          -156
#define IQ_8900           -157
#define IQ_8920           -158
#define IQ_9000           -159
#define IQ_9000MK2        -160
#define IQ_9200           -161
#define IQ_9200MK2        -162
#define ZQ_1000           -163
#define ZQ_1050           -164
#define ZQ_1200           -165
#define ZQ_1250A          -166
#define ZQ_2200           -167
#define ZQ_2400           -168
#define ZQ_2750           -169
#define ZQ_2500           -170
#define ZQ_3000           -171
#define ZQ_3200           -172
#define ZQ_3250           -173
#define ZQ_4450           -174
#define ZQ_5000           -175
#define ZQ_5100M          -176      
#define ZQ_5300M          -177
#define ZQ_6000           -178
#define ZQ_6100M          -179
#define ZQ_6200           -180
#define ZQ_6300M          -181
#define XE_7000           -182
#define ZQ_2450           -183
#define DO_PRINT          -185
#define DO_GLOSSARY       -186
#define DO_REVIEW_PRINT   -187
#define DOS_SHELL         -188
#define MENU_TREE         -189
#define PREVIOUS_FILES    -190
#define IQ_8500M          -191
#define IQ_8600M          -192

struct menu_def
{
   int start_x,
       start_y;
   int width,
       number_options;
   int attrib[MAX_MENU_OPTIONS];
   char option[MAX_MENU_OPTIONS][MAX_OPTION_SIZE+1];
};



