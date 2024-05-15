#define IQ_OFFLINE        0        // IQ either offline or not connected     
#define IQ_SUCCESS        1        // Succesful execution of function        
#define INVALID_PORT      2        // Only COM1 & COM2 supported             
#define IQ7000            4        // ZQ / IQ-7000 specifier                 
#define IQ8000            3        // IQ-8000 specifier                      
#define IQ9000            10       // IQ-9000 Series Specifier
#define IQ8900            11
#define CHECKSUM_ERROR    5        // Data checksum error                    
#define FILE_ERROR        6        // Error opening file                     
#define MODE_ERROR        7
#define INVALID_PARAM     8
#define NO_MEMORY         9
#define READ_ERROR        10   

#define RCV_TIMEOUT       -1       // Timed out waiting for a char form IQ  
#define MAX_RECV_WAIT     10       // Max time to wait for character         
#define ERROR_END         0x0a     // Error in session                       
#define NORMAL_END        0x06     // Session OK                             
#define NORMAL_END_R      0x02    
#define EOB               0x1A     // End of block marker                    

#define MAIN_RAM_APPEND      '0'   // Target Main ram appending            
#define MAIN_RAM_OVERWRITE   '1'   // Target main RAM overwriting           
#define EXPAND_RAM_APPEND    '8'   // Target expansion RAM appending        
#define EXPAND_RAM_OVERWRITE '9'   // Target expansion RAM overwriting      
#define DATA_MODE_OFF 5            // Offset of data mode in info block1    
#define DATA_NAME_OFF 8            // Offset of data name in info block1    
#define BLK1_LEN 21                // Info block1 length     
               
#define ONLINE_DELAY    3     
#define MODE            ASINOUT|BINARY|NORMALRX
#define BUF_LEN         10000
#define WORD_LEN        8
#define STOP_BIT        1
#define BAUD            9600

#define RXBUFSIZE       256
#define TXBUFSIZE       256

