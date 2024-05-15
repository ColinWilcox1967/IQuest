#define PORT1           0
#define PORT2           1
#define PORT3           2
#define PORT4           3

#define PARITY_ODD      0
#define PARITY_EVEN     1
#define PARITY_NONE     2

#define STOP_BITS1      0
#define STOP_BITS2      1

#define BAUD_300        0
#define BAUD_1200       1
#define BAUD_2400       2
#define BAUD_4800       3
#define BAUD_9600       4

#define DATA_BITS7      0
#define DATA_BITS8      1

#define FLOW_HARDWARE    0
#define FLOW_XONXOFF     1

struct comms_def
{
   int port;
   int parity;
   int stop;
   int speed;
   int data;
   int flow;
}; 

