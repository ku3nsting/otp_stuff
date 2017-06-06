#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>

//Exit codes
#define SOMETHING_BAD_HAPPENED	1
#define YOURE_GOOD_BOO			0

//Variables
int pid;

//Error checker
void error(const char *msg){
	perror(msg);
	exit(SOMETHING_BAD_HAPPENED); 
	}
	

//********************************
//
// S I G N A L  H A N D L E R
// source: http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#simpleserver
//
//********************************
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

	
//********************************
//
// E N C R Y P T O R  function
//
//********************************
char* doEncryption(char message[], char key[]){
	  int i = 0;
	  char current;
	  char keyChar;
	  int encrypted;
	  
	  //iterate until you hit the end of plaintext, which is an @ symbol
	  while(message[i] != '\0'){
		  
	  		current = message[i];
			keyChar = key[i];
			
			//check for spaces
			if(message[i] == ' '){
				current = '@';
			}
			if(key[i] == ' '){
				keyChar = '@';
			}
			
			//make the chars ints
			int msg = (int)current;
			int keyInt = (int)keyChar;
			
			//subtract 64 (same as subtracting A)
			msg = msg - 64;
			keyInt = keyInt - 64;
			
			//add them together to get encrypted number
			encrypted = (msg + keyInt);
			
			//take mod 27 to get it in the range of possible chars
			encrypted = encrypted % 27;
			
			//add 64 (same as adding A), to get ascii letter code
			encrypted = encrypted + 64;
			
			//put this back into message[] as char
			current = encrypted;
			message[i] = current;
			
			//if what we wrote was a @, put a space there instead
			if(message[i] == '@'){
				message[i] = ' ';
			}
		
			//printf("ENCRYPTED int: %d, ENCRYPTED char: %c\n", current, current);
			//printf("ENCRYPTED int: %d, ENCRYPTED char: %c\n", current, current);
			
			//iterate i
			i++;
	  }
	  //message[i] == newline, so overwrite newline with null terminator
	  //message[i] = '\0';
	  
	  //return the completely encrypted, null-terminated message
	  return message;
}

//***************************************
//
// M A I N  function
// sets up the encryption server and 
// loops forever
//
//***************************************
int main(int argc, char *argv[]){
	int listenSocketFD;
	int establishedConnectionFD;
	int portNumber;
	int charsRead;
	socklen_t sizeOfClientInfo;
	char buffer[100000];
	struct sockaddr_in serverAddress;
	struct sockaddr_in clientAddress;

	//too few arguments:
	//only takes one argument: listening port
	if (argc < 2){
		fprintf(stderr, "USAGE: %s port\n", argv[0]);
		fprintf(stdout, "Not enough arguments!\n");
		exit(SOMETHING_BAD_HAPPENED);
		} // Check usage & args

	//*****************************************
	//
	// S E T  U P  A D D R E S S  struct for
	// this process (the server)
	//
	//*****************************************
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	
	serverAddress.sin_port = htons(portNumber); // Store the port number
	
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
	
	//printf("enc setup completed\n");

	//*****************************************
	//
	// S E T  U P  socket
	//
	//*****************************************
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");
	
	//printf("enc socket setup completed\n");

	//*****************************************
	//
	// E N A B L E  socket to begin listening
	//
	//*****************************************
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections
	
	//printf("enc socket enabled completed\n");
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	
	//*************************
	// keepalive loop
	//*************************
	while(1) {  // main accept() loop

	//*****************************************
	//
	// A C C E P T  C O N N E C T I O N
	// blocking if one is not available until
	// one connects
	//
	//*****************************************
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0){
		error("ERROR on accept");
		continue;
	}
	
	//printf("enc connection accepted\n");
	
	// when we get a new connection, fork it up:
    pid = fork();

    if (pid < 0){
            perror("ERROR: Encryption server fork error\n");
			}
    // otherwise, we're in the child process
    if (pid == 0){

	//*****************************************
	//
	// G E T  M E S S A G E  from client
	//
	//*****************************************
	
	//printf("working on message\n");
	
	// Get the message from the client and display it
	memset(buffer, '\0', 100000);
	charsRead = recv(establishedConnectionFD, buffer, 99999, 0); // Read the client's message from the socket
	if (charsRead < 0){
		error("ERROR reading from socket");
	}
	
	//printf("SERVER: I received stuff from the client\n");
	
	//make strings to hold data
	char plainPlus[2048] = {0};
	char* ptr;
	char plaintext[2048] = {0};
	char key[100000] = {0};
	char encryptedText[2048];
	
	//check that this info is intended for encryption
	if(buffer[0] == 'E' && buffer[1] == 'N' && buffer[2] == 'C'){
		//encryption requested!
		//printf("ENCRYPTION REQUESTED!\n");
		
		ptr = strtok(buffer, "\n");
		sprintf(plainPlus, "%s", ptr);
		
		//plainPlus has 3 unwanted letters at the beginning
		strncpy(plaintext, plainPlus+3, sizeof(plainPlus)-3);
		//printf("plain before cut: %s", plainPlus);
		//printf("final plaintext: %s", plaintext);
		
		ptr = strtok(NULL, "@");
		sprintf(key, "%s", ptr);
		//printf("key: %s", key);
		
		//now that we have our key and our message, we can do the encryption magic:
		//printf("old plaintext: %s\n", plaintext);
		char* newPointer = doEncryption(plaintext, key);
		//printf("ENCRYPTION: %s", newPointer);
		sprintf(encryptedText, "%s", newPointer);
	}
	else{
		//exit because we didn't request the right server
		printf("ERROR: Wrong server selected\n");
		exit(1);
	}
	
	//check that string contains valid characters
	if((buffer[3] >= 'A' && buffer[3] <= 'Z') || buffer[3] == ' '){
		//do nothing, the message is probably valid
	}
	else{
		//exit because the message is garbage
		printf("ERROR: Invalid characters in plaintext\n");
		exit(1);
	}
	

	// Send a Success message back to the client
	charsRead = send(establishedConnectionFD, encryptedText, strlen(encryptedText), 0); // Send success back
	
	//handle error
	if (charsRead < 0){
		error("ERROR writing to socket");
	}
	
	// Do I need to close sockets within child?
    close(establishedConnectionFD);
    close(listenSocketFD);
    exit(0);
	
	}
	else{
		close(establishedConnectionFD);
	}
	
	} //this closes the while loop
	
	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	
	return YOURE_GOOD_BOO; 
}
