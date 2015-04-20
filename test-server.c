#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#define CPORT 59079
#define SPORT 59080

#define STORAGE "Store"

static float LOSS_RATE=0.0, ERR_RATE=0.0;

/* uncomment this part for part 1
#include "rdt-part1.h"
#define MSG_LEN PAYLOAD
*/

/* uncomment this part for part 2 
#include "rdt-part2.h"
#define MSG_LEN PAYLOAD
*/

/* uncomment this part for part 3 */
#include "rdt-part3.h"
#define MSG_LEN PAYLOAD*W



int main(int argc, char *argv[]) {

	int sockfd;
	char filepath[200]; 
	char * s;
	char msg[MSG_LEN];
	struct stat sbuf;	
	FILE * testfile;
	int file_len, len;
	int received=0;
	
	if (argc != 2) {
		printf("Usage: %s 'client hostname'\n", argv[0]);
		exit(0);
	}
	
	/* update random seed */
	srand(time(NULL));
	/* remove the above line if you want to get the same random 
	   sequence for each run - good for testing */
	
	/* check whether the folder exists */
	if (stat(STORAGE, &sbuf) != 0) {
		printf("Directory ./%s does not exist!!\n", STORAGE);
		printf("Please create the directory %s before start up the server.\n", STORAGE);
		exit(0);
	}
		
	/* read in packet loss rate and error rate */
	s = getenv("PACKET_LOSS_RATE");
	if (s != NULL) LOSS_RATE = strtof(s, NULL);
	s = getenv("PACKET_ERR_RATE");
	if (s != NULL) ERR_RATE = strtof(s, NULL);
	printf("PACKET_LOSS_RATE = %.2f, PACKET_ERR_RATE = %.2f\n", LOSS_RATE, ERR_RATE);
	
	// create RDT socket
    sockfd = rdt_socket();
    
    //specify my own IP address & port number, because if I do not specify, others can not send things to me.
	rdt_bind(sockfd, SPORT);    

	//specify the IP address & port number of my partner 
    rdt_target(sockfd, argv[1], CPORT);
    
	/* a very simple handshaking protocol */
	// wait for client request
	memset(msg, '\0', MSG_LEN);
	len = rdt_recv(sockfd, msg, MSG_LEN);
	file_len = atoi(msg);
	printf("Received client request: file size = %d\n", file_len);
	
	memset(msg, '\0', MSG_LEN);
	len = rdt_recv(sockfd, msg, MSG_LEN);
	sprintf(filepath, "%s/%s", STORAGE, msg);
	testfile = fopen(filepath, "w");
	if (!testfile) {
		printf("Cannot open the target file: ./%s for write\n", filepath);
		
		// send the ERROR response
		memset(msg, '\0', MSG_LEN);
		sprintf(msg, "ERROR");
		rdt_send(sockfd, msg, strlen(msg));
		goto END;
	} else {
		printf("Open file %s for writing successfully\n", filepath);
		// send the ERROR response
		memset(msg, '\0', MSG_LEN);
		sprintf(msg, "OKAY");
		rdt_send(sockfd, msg, strlen(msg));
	}

	/* start the file transfer */   
	printf("Start receiving the file . . .\n");
    // receive the file contents
    while (received < file_len) {
    	memset(msg, 0, MSG_LEN);
    	len = rdt_recv(sockfd, msg, MSG_LEN);
    	fwrite(msg, sizeof(char), len, testfile);
    	received += len;
    	printf("Received a message of size %d bytes\n", len);
    }

	printf("Complete the file transfer.\n");
	
END:    
    // close the file
    fclose(testfile);
    
    // close the rdt socket
    rdt_close(sockfd);
    
    printf("Server program terminated\n");
    
    return 0;
}

