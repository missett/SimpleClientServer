#ifndef CONNECTION_H
#define CONNECTION_H

#define PORT_NUM	2000
#define TEMP_POR	3000
#define SERVERIP	"127.0.0.1"
#define BUFF_LEN	256

#define OPEN_CONN	0x01 //Request a new connection
#define GOOD_CONN	0x02 //Connection is ok
#define CLOS_CONN	0x04 //Tell server to shut connection
#define SHUT_CONN	0x08 //Server has shut connection
#define HAS_DATA	0x10 //Packet contains data for use

//Total size of Packet should be 256 bytes
typedef struct myPacket {
	u_char header;
	char data[255];
} Packet;

/*Universal functions*/
int Connect ( char * ipAddress ); //Connects to the server
int Disconnect ( void ); //Disconnects from the server
int Send ( char * data , int length ); //Sends a packet of data
int Accept ( void ); //Waits for connection attempts
int Recv ( char * buffer , int maxlength ); //Waits for a data packet
void Stop ( void ); //Waits for disconnection
void die(char * error); //Used to deal with fatal errors

void setup( void );
void replyToConnectionAttempt(char * ipAddress);

/*Functions for the client*/
Packet buildRequestConnectionPacket ( void );
Packet buildRequestClosePacket( void );
Packet buildDataPacket( char * data );

/*Functions for the server*/
Packet buildOpenConnectionPacket( void );
Packet buildShutConnectionPacket( void );

#endif
