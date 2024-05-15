/* Timer support */
#define TIMER_STOPPED -1

typedef struct                         /* structure to hold timer details */
{
    char stamp;
    int  last_time;                    /* Time when last looked at */
    int  to_go;                        /* Time to expiration */
} 
timer_info;


unsigned char info_block1[1024];   // 21 bytes block * 48 Application names
unsigned char info_blk_user[21];   // 21 bytes for user specified block
int userdef;                       // MJG 20/11/91
