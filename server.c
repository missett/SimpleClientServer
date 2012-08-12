#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include "connection.h"

//Track the state of the server
//0 = Awaiting connection
//1 = Ready for data transmision
int state;

//Socket identifier
int socketID; 

//Buffer for receiving data
char buffer[BUFF_LEN];

//Store details of client and server
struct sockaddr_in clientAddress;
struct sockaddr_in serverAddress;

int main ( void ) {
	state = 0;

	setup();

	Accept();

	Recv(buffer , BUFF_LEN);

	return 1;
}

void setup( void ) {
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(PORT_NUM);
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	if( ( socketID = socket( AF_INET , SOCK_DGRAM , IPPROTO_UDP ) ) == -1 )
		die("Socket Setup Error");
	
	struct sockaddr_in * serverAddress_ptr = &serverAddress;
	struct sockaddr * serverAddress_ptr1 = (struct sockaddr *) serverAddress_ptr;

	if( bind( socketID , serverAddress_ptr1 , sizeof(serverAddress) ) == -1 )
		die("Socket Binding Error");
}

Packet buildOpenConnectionPacket( void ) {
	Packet newPacket;
	memset( &newPacket , 0 , sizeof(Packet) );

	newPacket.header = GOOD_CONN; 
	
	return newPacket;
}

Packet buildShutConnectionPacket( void ) {
	Packet newPacket;
	memset( &newPacket , 0 , sizeof(Packet) );

	newPacket.header = SHUT_CONN; 
	
	return newPacket;
}

void respondToConnectionAttempt ( char * ipAddress ) {
	//Pointers for data sending
	Packet responsePacket = buildOpenConnectionPacket();
	Packet * pack_ptr = &responsePacket;
	char * ptr = (char *) pack_ptr; 
	
	//Build up address of destination
	//struct sockaddr_in * serverAddress_ptr = &serverAddress;
	//struct sockaddr * serverAddress_ptr1 = (struct sockaddr *) serverAddress_ptr;
	
	//We use this struct to reply because it has the reply details already filled out
	struct sockaddr_in * clientAddress_ptr = &clientAddress;
	struct sockaddr * clientAddress_ptr1 = (struct sockaddr *) clientAddress_ptr;

	socklen_t length = sizeof(struct sockaddr_in);

	if( sendto(socketID , ptr , BUFF_LEN , 0 , clientAddress_ptr1 , length) == -1 )
		die("Could not reply");
}

void die ( char * error ) {
	perror(error);
	exit(0);
}

int Accept ( void ) {
	struct sockaddr_in * clientAddress_ptr = &clientAddress;
	struct sockaddr * clientAddress_ptr1 = (struct sockaddr *) clientAddress_ptr;

	socklen_t length = sizeof(struct sockaddr);

	while (state<1) {
		memset(buffer , 0 , BUFF_LEN); //Clear the buffer of the last message received

		if( recvfrom( socketID , buffer , BUFF_LEN , 0 , clientAddress_ptr1 , &length ) == -1 )
			die("Could not receive packet");

		char * str_ptr = &buffer[0];
		Packet * pack_ptr = (Packet *) str_ptr;

		if( pack_ptr->header == OPEN_CONN ) {
			//Get the IP address that is trying to communicate
			char ipBuffer[INET_ADDRSTRLEN];
			inet_ntop( AF_INET , &clientAddress.sin_addr , ipBuffer , INET_ADDRSTRLEN );
			
			printf("Connection Requested By %s\nResponding...\n" , ipBuffer);
			
			sleep(3); //Sim a real network connection

			//Send a response back
			respondToConnectionAttempt(ipBuffer);

			state++;
		}
		
		//Function exits with state set to accept incoming data packets
	}

}

int Recv ( char * buffer , int maxLength ) {
	//Structs for packet handling
	struct sockaddr_in * clientAddress_ptr = &clientAddress;
	struct sockaddr * clientAddress_ptr1 = (struct sockaddr *) clientAddress_ptr;

	socklen_t length = sizeof(struct sockaddr);

	while ( state>0 ) {
		memset(buffer , 0 , maxLength);

		if( recvfrom( socketID , buffer , maxLength , 0 , clientAddress_ptr1 , &length ) == -1 )
			die("Error Receiving Data");
		
		char * buf_ptr = &buffer[0];
		Packet * pack_ptr = (Packet *) buf_ptr;

		if(pack_ptr->header == CLOS_CONN) {
			state--;
			//Optionally call Stop() from here
			Stop();
		} else {
			printf(">>%s\n" , pack_ptr->data);
		}

	}

}

void Stop ( void ) {
	puts("Client shutting connection...");

	Packet shutPacket = buildShutConnectionPacket();
	Packet * shut_ptr = &shutPacket;

	char * ptr = (char *) shut_ptr;

	struct sockaddr_in * clientAddress_ptr = &clientAddress;
	struct sockaddr * clientAddress_ptr1 = (struct sockaddr *) clientAddress_ptr;

	socklen_t length = sizeof(struct sockaddr);

	if( sendto( socketID , ptr , BUFF_LEN , 0 , clientAddress_ptr1 , length ) == -1)
		die("Could not send shutdown packet");

	close(socketID);
	puts("Connection shut. Bye.");
}
