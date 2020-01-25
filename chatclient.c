/*******************************************************
** Author: Daniel Yopp 
** Date: 7/26/19
** Program Name: ChatClient
** Description: A chat client that connects to the chatserver enable communication 
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

////////////////////////////////////////////////////////
//Name: addTail
//Description: Adds a tail of "@@@" to the cstring parameter
//			   Risk of buffer overflow if cstring is completely full
//			   Not possible in this program the way the buffer is malloced	
////////////////////////////////////////////////////////
void addTail(char* buff)
{	
	int stringLength = strlen(buff); //counts char not including '\0'
	buff[stringLength] = '@';
	buff[stringLength+1] = '@';
	buff[stringLength+2] = '@';
	buff[stringLength+3] = '\0';
}



///////////////////////////////////////////////////////
// Name: CheckTail
// Description: Checks a cstring for a @@@ tail
// Returns 1 if not @@@; Returns 0 if @@@ is present 
///////////////////////////////////////////////////////
int checkTail(char* buff)
{
	int strLen = strlen(buff) - 2;
	int j = 0;
	for(j = 0; j < strLen; j++)
	{
		if (buff[j] == '@')
		{
		   if (buff[j+1] == '@' && buff[j+2] == '@')
		   	{
			   buff[j+3] = '\0';
			   return 0;	
			}
		}
	}
	return 1;
}

///////////////////////////////////////////////////////
// Name: RemoveTail
// Description: if last 3 chars in cstring = @@@ then they are removed from the cstring
///////////////////////////////////////////////////////
void removeTail(char* buff)
{
	if(checkTail(buff)==0)
	{
	   int strLen = strlen(buff);
	   buff[strLen-3] = '\0';	
	}
}


//////////////////////////////////////////////////////
// Name: ChatCycle
// Description: Sends client message than recieves
//  server message in loop until user prompted quit
/////////////////////////////////////////////////////
void ChatCycle( int socketFD, char* serverName, char* clientName )
{

   size_t bufsize = 500; //size of input allowed, must be 4 less than size of array to allow for @@@\n tail

   char* buffer= malloc(sizeof(char[bufsize+4]));	
   while(1)
   {	
		int charsWritten;
		

		//gather client message input
		memset(buffer, '\0', 504); //clear buffer
		printf("%s> ", clientName); //print client username as prompt
		getline(&buffer, &bufsize , stdin); //get user input(getline includes \n
			//will leave anything longer than bufsize in stdin- consider flushing?

		//check if client wants  to terminate chat
		if(strcmp("\\quit\n", buffer) == 0)
			{printf("---Client Termination Request\n");
			addTail(buffer); // add @@@ tail
			charsWritten = send(socketFD, buffer, strlen(buffer), 0); //send term notice to server
			if (charsWritten < 0) {fprintf(stderr, "Client: ERROR sending message"); exit(0);}
			if (charsWritten < strlen(buffer)) {printf("Client: Warning! message fragmented!");}
			fflush(stdout); 
			break; //get out of while loop
			}

		//send client message
		addTail(buffer);// add @@@ tail
		charsWritten = send(socketFD, buffer, strlen(buffer) , 0); //send to server
		if (charsWritten < 0) {fprintf(stderr, "Client: ERROR sending message"); exit(0);}
		if (charsWritten < strlen(buffer)) {printf("Client: Warning! message fragmented!");}
		fflush(stdout);	

		//recieve server message
		printf("\n***Waiting on Server Message***\n");
		char holdingBuffer[504];
		int charsRead = 0;
		memset( holdingBuffer, '\0', sizeof(holdingBuffer)); //clear buffer for recieving server message

		while (checkTail(holdingBuffer) ) //checktail returns 1 if @@@ tail is not present
			{
			memset(buffer, '\0', sizeof(buffer));
			charsRead = recv(socketFD, buffer, sizeof(buffer)-1, 0);
			strcat(holdingBuffer, buffer);//copy whats received to the holding buffer
			}
		removeTail(holdingBuffer); //remove @@@ tail from server message

		//check if server requests term
		if(strcmp("\\quit", holdingBuffer) == 0)
			{printf("\n---Server Termination Request\n   Goodbye :)\n");
			fflush(stdout);
			break;}
		else
			{printf("\n%s> %s\n\n", serverName, holdingBuffer );
			fflush(stdout);
			}
   }//end of while loop
   free(buffer);//release mem		
}


////////////////////////////////////////////////////
// Main Function
/////////////////////////////////////////////////// 
int main(int argc, char *argv[])
{
	//check args input
	if(argc != 3)
	{   fprintf(stderr, "ERROR: Incorrect Command Line Arguments!\n");
 	    fprintf(stderr, "Supply two arguments: <host> <port>\n");
	    exit(1);	}
	 
	//create a client username
	char clientName[11];
	clientName[10] = 'x';
	while(clientName[10] != '\0') //if user enters name that overflows cstring then reprompt
	{	memset(clientName, '\0', 11);
		printf("Enter a username (no longer than 10 chars): ");
		scanf ("%s", clientName);
		while((getchar())!='\n'); //clear newline char out of stdin for future getline 
		if (clientName[10] != '\0')
		{   printf("ERROR: Client name too long!\n");}
	}

	//socket declarations
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[501];

	//set up server connection
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); //clear address struct
	portNumber = atoi(argv[2]); //get port and convert to int
	serverAddress.sin_family = AF_INET; //create network capable socket
	serverAddress.sin_port = htons(portNumber); //store the port
	serverHostInfo = gethostbyname(argv[1]); //convert machine name into special form of address
	  if(serverHostInfo == NULL) {fprintf(stderr, "Client: ERROR, no such host"); exit(0);}
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length);
	socketFD = socket(AF_INET, SOCK_STREAM, 0); //set up socket
	  if (socketFD < 0) {fprintf(stderr, "Client: ERROR opening socket"); exit(0);}

	//connect to server - connect socket to address
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0)
	{fprintf(stderr, "Client: ERROR connecting"); exit(0);}

	//send client username
	charsWritten = send(socketFD, clientName, strlen(clientName), 0);
	if (charsWritten < 0) {fprintf(stderr, "Client: ERROR sending username"); exit(0);}
	if (charsWritten < strlen(buffer)) {printf("Client: Warning! USername fragmented!");}

	//get server username
	char serverName[11];
	memset( serverName, '\0', sizeof(serverName));
	charsRead = recv(socketFD, serverName, sizeof(serverName)-1, 0);
	if (charsRead < 0) {fprintf(stderr, "Client: Error recieving servername"); exit(0);} 
	
	printf("Connection Established. Ready to Chat!\n\n");
	printf("Directions: Type a msg up to 500 char.\nThen wait for response.\nType \"\\quit\" to end connection\n\n"); 	

	//chat cycle
	ChatCycle( socketFD, serverName, clientName);

	close(socketFD);

	return 0;

}	


