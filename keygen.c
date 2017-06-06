//********************************************************
// K E Y G E N
// keygen.c
// CS 344
// May 31, 2017
//*******************************************************

//*******************************************************
//keygen: This program creates a key file of specified length. The characters in the file generated will be any of the 27 allowed characters, generated using the standard UNIX randomization methods. Do not create spaces every five characters, as has been historically done. Note that you specifically do not have to do any fancy random number generation: we’re not looking for cryptographically secure random number generation! rand() is just fine. The last character keygen outputs should be a newline. All error text must be output to stderr, if any.
//*******************************************************
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

//Exit codes:
#define HAPPY_ENDING 	0
#define EVIL_PREVAILS 	1

void keygen(int keylength){
	
	int i;
	//printf("\nin function, keylength = %d", keylength);
	
	char keystring[99999] = {0};
	
	for(i = 0; i < keylength; i++){
		
		//get a random number 0-26 (inclusive)
		int randInt = rand() % 27;
		
		//start with letter == space
		char letter = ' ';
		
		//if random==any number other than 26, overwrite letter with corresponding char
		if(randInt < 26){
			letter = ('A' + randInt);
		}
		//I was worried aboutt his edge case, not sure why
		else if(randInt == 27){
			printf("Out-of-range key generated. Try again.");
			letter = ('#');
			exit(EVIL_PREVAILS);
		}
		
		sprintf(keystring, "%s%c", keystring, letter);
		//printf("Current string: %s", keystring);
	}
	sprintf(keystring, "%s%c", keystring, '\n');
	printf("%s", keystring);
	
	//make a file
	FILE *fp = fopen("keygen.txt", "w");
	if (fp == NULL){
		printf("Error opening file! Exiting.");
		exit(EVIL_PREVAILS);		
	}
	
	fprintf(fp, "%s", keystring);
	
	fclose(fp);
}

int main(int argc, char *argv[]){
	
	//time seed for (srand)
	srand(time(NULL));
	//printf("time seed\n");

	//placeolder for length variable
	int length = 0;
	
	//deal with arguments
	if( argc == 2 ){
	  length = atoi(argv[1]);
      //printf("The argument is %d\n", length);
	}
    else if( argc > 2 ){
      printf("Too many arguments.\n");
	}
    else{
      printf("This program must take one argument.\n");
	}
  
	//printf("calling keygen with %d", length);
	keygen(length);
	
	return(HAPPY_ENDING);
}