/*
 *Program name: Server-Client program
 *Author: Maria Wiklund and Gina Senewiratne
 * 
 *General Description:
 *This program generates a connection between the Client and server side.
 *It shows how socket communication works between Server and one or multiple Clients.
 */



/* File: server.c
 * Trying out socket communication between processes using the Internet protocol family.
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#define PORT 5555
#define MAXMSG 512

/* makeSocket
 * Creates and names a socket in the Internet
 * name-space. The socket created exists
 * on the machine from which the function is 
 * called. Instead of finding and using the
 * machine's Internet address, the function
 * specifies INADDR_ANY as the host address;
 * the system replaces that with the machine's
 * actual address.
 */
int makeSocket(unsigned short int port) {
  int sock;
  struct sockaddr_in name;

  /* Create a socket. */
  sock = socket(PF_INET, SOCK_STREAM, 0);
  if(sock < 0) {
    perror("Could not create a socket\n");
    exit(EXIT_FAILURE);
  }
  /* Give the socket a name. */
  /* Socket address format set to AF_INET for Internet use. */
  name.sin_family = AF_INET;
  /* Set port number. The function htons converts from host byte order to network byte order.*/
  name.sin_port = htons(port);
  /* Set the Internet address of the host the function is called from. */
  /* The function htonl converts INADDR_ANY from host byte order to network byte order. */
  /* (htonl does the same thing as htons but the former converts a long integer whereas
   * htons converts a short.) 
   */
  name.sin_addr.s_addr = htonl(INADDR_ANY);
  /* Assign an address to the socket by calling bind. */
  if(bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
    perror("Could not bind a name to the socket\n");
    exit(EXIT_FAILURE);
  }
  return(sock);
}

/* readMessageFromClient
 * Reads and prints data read from the file (socket
 * denoted by the file descriptor 'fileDescriptor'.
 */
int readMessageFromClient(int fileDescriptor) {
  char buffer[MAXMSG];
  int nOfBytes;

  nOfBytes = read(fileDescriptor, buffer, MAXMSG);
  if(nOfBytes < 0) {
    perror("Could not read data from client\n");
    exit(EXIT_FAILURE);
  }
  else
    if(nOfBytes == 0) 
      /* End of file */
      return(-1);
    else 
      /* Data read */
      printf(">Incoming message: %s\n",  buffer);
  return(0);
}


/* LAB 2 PART 1 
 * Writes a message to a client
 */
void writeMessageToClient(int clientSocket, char msg[])
{
	int nOfBytes = send(clientSocket, msg, strlen(msg)+1, 0);
	if(nOfBytes < 0){
		perror("Could not send reply");
		exit(EXIT_FAILURE);
	}	
}


/*LAB 2 PART 3
 * Sends a broadcast to all connected clients
 */

void broadcast(const char *msg)
{
	struct sockaddr_in addr;
	int UDPsocket;
	//use of an arbitrary port
	int port = 12345;
	char ip[] = "255.255.255.255";

	UDPsocket = socket(PF_INET, SOCK_DGRAM, 0);
	if(UDPsocket < 0){
		perror("Could not create a socket");
		exit(EXIT_FAILURE);
	}
	
	//for allowing broadcasts
	int broadcastOK = 1;
	int optCheck = setsockopt(UDPsocket, SOL_SOCKET, SO_BROADCAST, &broadcastOK, sizeof(broadcastOK));
	if(optCheck < 0){
		perror("Could not set options");
		exit(EXIT_FAILURE);
	}
	
	//set up destination address
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = PF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	if(sendto(UDPsocket, msg, strlen(msg+1), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		perror("could not send message");
		exit(EXIT_FAILURE);
	} 
	else printf("Broadcast sent to all clients\n\n");
}


/* LAB 2 PART 4
 * Refusing a client IP-address by shutting down the connection
 */

void refuseIP(int clientSocket)
{
	writeMessageToClient(clientSocket, "You are refused");
	
	//disallow transmissions and receptions
	shutdown(clientSocket, SHUT_RDWR);
	//read from socket until error or connection closed, then close connection.
	char buffer[100];
	if(recv(clientSocket, buffer, strlen(buff+1), 0) < 1){
		close(clientSocket);
		//socket closed
	}
}

/*---------END OF LAB 2------------------------------------*/

int main(int argc, char *argv[]) {
  int sock;
  int clientSocket;
  int i;
  fd_set activeFdSet, readFdSet; /* Used by select */
  struct sockaddr_in clientName;
  socklen_t size;
  
 
  /* Create a socket and set it up to accept connections */
  sock = makeSocket(PORT);
  /* Listen for connection requests from clients */
  if(listen(sock,1) < 0) {
    perror("Could not listen for connections\n");
    exit(EXIT_FAILURE);
  }
  /* Initialize the set of active sockets */
  FD_ZERO(&activeFdSet);
  FD_SET(sock, &activeFdSet);
  
  printf("\n[waiting for connections...]\n");
  while(1) {
    /* Block until input arrives on one or more active sockets
       FD_SETSIZE is a constant with value = 1024 */
    readFdSet = activeFdSet;
    if(select(FD_SETSIZE, &readFdSet, NULL, NULL, NULL) < 0) {
      perror("Select failed\n");
      exit(EXIT_FAILURE);
    }
    /* Service all the sockets with input pending */
    for(i = 0; i < FD_SETSIZE; ++i) 
      if(FD_ISSET(i, &readFdSet)) {
	if(i == sock) {
	  /* Connection request on original socket */
	  size = sizeof(struct sockaddr_in);
	  /* Accept the connection request from a client. */
	  clientSocket = accept(sock, (struct sockaddr *)&clientName, (socklen_t *)&size); 
	  if(clientSocket < 0) {
	    perror("Could not accept connection\n");
	    exit(EXIT_FAILURE);
	  }


	  /*LABB 2 PART 4 Refuse a connection from a hardcoded client*/
	  else if(clientName.sin_addr.s_addr != inet_addr("127.0.0.1")){
		printf("Shutting down a refused connection..\n");
		refuseIP(clientSocket);	
	  }
	  else{
		
	  /*---------------END OF LAB 2------------------------i-*/
	  printf("Server: Connect from client %s, port %d\n", 
		 inet_ntoa(clientName.sin_addr), 
		 ntohs(clientName.sin_port));
	
/*LAB 2 Part 3 Broadcast to all clients that there is a new client connected*/
	char broadcastMsg[] = "Hey guys, looks like we have a new friend!\n";
	broadcast(broadcastMsg);
/*-----------------------------END OF LAB 2 -------------------------------*/	  
	  
	
	
	FD_SET(clientSocket, &activeFdSet);

	  }
	}
	else {
	  /* Data arriving on an already connected socket */


	 /*---LAB 2 PART 1: Sending a reply to the client when server gets input--*/
	    	  char msg[] = "I hear you, dude...\n";
		  writeMessageToClient(clientSocket, msg);
	  
	/*--------------------------END OF LAB 2----------------------*/



	if(readMessageFromClient(i) < 0) {
	    close(i);
	    FD_CLR(i, &activeFdSet);
	  }
	}
      }
  }
}
