#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define CPORT 59079
#define SPORT 59080

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



int main(int argc, char *argv[]){

	int sockfd;
	FILE * testfile;
	int filelength, len;
	char * fname, * s;
	char msg[MSG_LEN];
	int sent = 0;
	struct timeval starttime, endtime;
	double lapsed;
	
	if (argc != 3) {
		printf("Usage: %s 'server hostname' 'filename'\n", argv[0]);
		exit(0);
	}	

	/* update random seed */
	srand(time(NULL));
	/* remove the above line if you want to get the same random 
	   sequence for each run - good for testing */
	
	/* read in packet loss rate and error rate */
	s = getenv("PACKET_LOSS_RATE");
	if (s != NULL) LOSS_RATE = strtof(s, NULL);
	s = getenv("PACKET_ERR_RATE");
	if (s != NULL) ERR_RATE = strtof(s, NULL);
	printf("PACKET_LOSS_RATE = %.2f, PACKET_ERR_RATE = %.2f\n", LOSS_RATE, ERR_RATE);

	fname=argv[2];
	//open file
	if (!(testfile = fopen(fname, "r"))) {
		printf("Open file failed.\nProgram terminated.");
		exit(0);
	}
	printf("Open file successfully \n");
	//get the file size
	fseek(testfile, 0L, SEEK_END); 
	filelength = ftell(testfile); 	
	printf("File bytes are %d \n",filelength);
	fseek(testfile, 0L, SEEK_SET);
		
	// create RDT socket
    sockfd = rdt_socket();
    
    //specify my own IP address & port number, because if I do not specify, others can not send things to me.
	rdt_bind(sockfd, CPORT);     
    
	//specify the IP address & port number of my partner 
    rdt_target(sockfd, argv[1], SPORT);

    /* a very simple handshaking protocol */ 
    //send the size of the file
    memset(msg, '\0', MSG_LEN);
    sprintf(msg, "%d", filelength);
    rdt_send(sockfd, msg, strlen(msg));  
      
    //send the file name to server
    rdt_send(sockfd, fname, strlen(fname));
    
	//wait for server response
    memset(msg, '\0', MSG_LEN);
	len = rdt_recv(sockfd, msg, MSG_LEN);
	if (strcmp(msg, "ERROR") == 0) {
		printf("Server experienced fatal error.\nProgram terminated.\n");
		goto END;
	} else
		printf("Receive server response\n");

	/* start the data transfer */
	printf("Start the file transfer . . .\n");
	gettimeofday(&starttime, NULL);
    // send the file contents
    while (sent < filelength) {
    	if ((filelength-sent) < MSG_LEN)
    		len = fread(msg, sizeof(char), filelength-sent, testfile);
    	else
    		len = fread(msg, sizeof(char), MSG_LEN, testfile);
    	rdt_send(sockfd, msg, len);
    	sent += len;
		usleep(1000);
    }
	gettimeofday(&endtime, NULL);
	printf("Complete the file transfer.\n");
	lapsed = (endtime.tv_sec - starttime.tv_sec)*1.0 + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
	printf("Total elapse time: %.3f s\tThroughtput: %.2f KB/s\n", lapsed, filelength/lapsed/1000.0);
	
END:    
    // close the file
    fclose(testfile);
    
    // close the rdt socket
    rdt_close(sockfd);
	printf("Client program terminated\n");

    return 0;
}

