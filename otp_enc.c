#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { 
	perror(msg); exit(0); 
	} // Error function used for reporting issues

int main(int argc, char *argv[]){
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[100000] = {0};
    
	//takes 3 arguments (plaintext, key, port)
	if (argc < 4) { fprintf(stderr,"USAGE: %s plaintext, key, port\n", argv[0]); exit(0); 
	}
	
	//for testing
	//printf("ARGUMENT 1: %s\n", argv[1]);
	//printf("ARGUMENT 2: %s\n", argv[2]);
	//printf("ARGUMENT 3: %s\n", argv[3]);
	
	//get length of key from keyfile:
	//got help from this stackoverflow convo: https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c
	//first, get key from file:
	FILE *keyfile;
	keyfile = fopen(argv[2], "r");
	int length;
	char keyBuffer[99999] = {0};
	if(!keyfile){
		perror(argv[2]);
		exit(1);
	}
	else{
		fseek(keyfile, 0, SEEK_END);
		length = ftell(keyfile);
		fseek(keyfile, 0, SEEK_SET);
		if(keyBuffer){
			fread(keyBuffer, 1, length, keyfile);
		}
		fclose(keyfile);
	}
	
	//get length of plaintext from textfile:
	//got help from this stackoverflow convo: https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c
	//first, get key from file:
	FILE *textfile;
	textfile = fopen(argv[1], "r");
	int lengtht;
	char txtBuffer[2048] = {0};
	if(!textfile){
		perror(argv[1]);
		exit(1);
	}
	else{
		fseek(textfile, 0, SEEK_END);
		lengtht = ftell(textfile);
		fseek(textfile, 0, SEEK_SET);
		if(txtBuffer){
			fread(txtBuffer, 1, lengtht, textfile);
		}
		fclose(textfile);
	}
	
	//compare sizes of arguments:
	char keyInput[99999] = {0};
	strcpy(keyInput, keyBuffer);
	//printf("LENGTH: %d\n",length);
	//printf("KEY: %s\n", keyBuffer);
	
	char plaintextInput[2048] = {0};
	strcpy(plaintextInput, txtBuffer);
	//free(txtBuffer);
	//printf("size of plaintext: %d\n", lengtht);
	//printf("TEXT: %s\n", txtBuffer);
	
	if(length < lengtht){
		fprintf(stderr,"ERROR: Key is too short to create a cypher. Try again.");
		exit(1);
	}
	
	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	
	serverAddress.sin_port = htons(portNumber); // Store the port number
	
	serverHostInfo = gethostbyname("localhost");
	
	if (serverHostInfo == NULL){
		fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); 
		}
		
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	// Create the socket
	if (socketFD < 0){
		error("CLIENT: ERROR opening socket");
	}
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
	// Connect socket to address
		error("CLIENT: ERROR connecting");
	}

	// Make a message to send to server
	char sendMessage[100000] = {0};
	snprintf(sendMessage, 99999, "ENC%s%s@", plaintextInput, keyInput);
	
	//printf("LENGTH OF SENDMESSAGE: %d", strlen(sendMessage));
	//printf("SENDING THIS MESSAGE: %s\n", sendMessage);

	// Send message to server
	charsWritten = send(socketFD, sendMessage, strlen(sendMessage), 0); // Write to the server
	
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(sendMessage)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	
	//printf("SENT stuff to socket sucessfully!");

	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	
	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	fprintf(stdout,"%s\n", buffer);

	close(socketFD); // Close the socket
	return 0;
}
