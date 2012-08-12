#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "connection.h"

//Track the state of the client
//0 = Attempting to connect
//1 = Ready to send data
int state;

//Socket identifier
int socketID;

//Store details of server and client
struct sockaddr_in serverAddress;
struct sockaddr_in clientAddress;

//Store data temporarily
char buffer[BUFF_LEN];

int main ( int argc , char * argv[] ) {
	state = 0;

	setup();

	Connect(SERVERIP);

	sleep(2);

	Send("Hello World" , 11); 
	Send("Ryan Missett" , 12); 

	sleep(2);

	Disconnect();
	
	return 1;
}


void setup( void ) {
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUM); //Used to be PORT_NUM
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if( ( socketID = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP ) ) == -1 )
		die("Socket Setup Error");
	
	struct sockaddr_in * serverAddress_ptr = &serverAddress;
	struct sockaddr * serverAddress_ptr1 = (struct sockaddr *) serverAddress_ptr;

}

Packet buildRequestConnectionPacket( void ) {
	Packet newPacket;
	memset( &newPacket , 0 , sizeof(Packet) );

	newPacket.header = OPEN_CONN; 
	
	return newPacket;
}

Packet buildRequestClosePacket( void ) {
	Packet newPacket;
	memset( &newPacket , 0 , sizeof(Packet) );

	newPacket.header = CLOS_CONN; 
	
	return newPacket;
}

Packet buildDataPacket( char * data ) {
	Packet newPacket;
	memset( &newPacket , 0 , sizeof(Packet) );

	newPacket.header = HAS_DATA; 
	sprintf( newPacket.data , "%s" , data );
	
	return newPacket;
}

void die ( char * error ) {
	perror(error);
	exit(0);
}

int Connect ( char * ipAddress ) {
	//Setup packet and pointers for sending
	Packet connect = buildRequestConnectionPacket();
	Packet * connect_ptr = &connect;
	char * ptr = (char *) connect_ptr;

	struct sockaddr_in * serverAddress_ptr = &serverAddress;
	struct sockaddr * serverAddress_ptr1 = (struct sockaddr *) serverAddress_ptr;

	socklen_t length = sizeof(struct sockaddr_in);

	if( sendto( socketID , ptr , BUFF_LEN , 0 , serverAddress_ptr1 , length ) == -1 )
		die("Sending Error");

	printf("Connecting with %s...\n" , ipAddress);

	while(state<1) {
		if( recvfrom(socketID , buffer , BUFF_LEN , 0 , serverAddress_ptr1 , &length) == -1)
			die("Problem Connecting");
		
		//Cast pointer to get received header
		ptr = &buffer[0];
		connect_ptr = (Packet *) ptr;

		if( connect_ptr->header == GOOD_CONN ) { //Is the received packet a connection auth?
			printf("Successfully connected to %s\n" , ipAddress);
			state++;
		}
	}
}

int Disconnect ( void ) {
	Packet endPacket = buildRequestClosePacket();
	Packet * pack_ptr = &endPacket;

	char * ptr = (char *) pack_ptr;

	struct sockaddr_in * serverAddress_ptr = &serverAddress;
	struct sockaddr * serverAddress_ptr1 = (struct sockaddr *) serverAddress_ptr;
	
	socklen_t length = sizeof(struct sockaddr_in);

	//Send off disconnect request
	if( sendto( socketID , ptr , BUFF_LEN , 0 , serverAddress_ptr1 , length ) == -1 ) 
		die("Problem Disconnecting");

	//Get back disconnect confirm
	if( recvfrom( socketID , buffer , BUFF_LEN , 0 , serverAddress_ptr1 , &length ) == -1 )
		die("Could not get shutdown packet");
	
	//Need to check packet is actually shutdown confirmation	

}

int Send ( char * data , int dataLength ) {
	Packet newPacket = buildDataPacket(data);
	Packet * pack_ptr = &newPacket;

	//strcpy(pack_ptr->data , data);
	//pack_ptr->header = HAS_DATA;

	char * ptr = (char *) pack_ptr;
	
	struct sockaddr_in * serverAddress_ptr = &serverAddress;
	struct sockaddr * serverAddress_ptr1 = (struct sockaddr *) serverAddress_ptr;
	
	socklen_t length = sizeof(struct sockaddr_in);
	
	//In sending we must correct for the length of the header so we add 1. Possibly.
	if( sendto( socketID , ptr , dataLength+1 , 0 , serverAddress_ptr1 , length ) == -1 )
		die("Could Not Send Data");
}

