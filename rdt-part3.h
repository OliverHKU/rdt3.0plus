/**************************************************************
rdt-part3.h
Student name: Li Wenjian
Student No. : 3035027497
Date and version: 15/4/2015 v2.0
Development platform: OS X
Development language: C
Compilation:
	Can be compiled with g++
*****************************************************************/

#ifndef RDT3_H
#define RDT3_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netdb.h>

#define PAYLOAD 1000		//size of data payload of the RDT layer
#define TIMEOUT 50000		//50 milliseconds
#define TWAIT 10*TIMEOUT	//Each peer keeps an eye on the receiving  
							//end for TWAIT time units before closing
							//For retransmission of missing last ACK
#define W 9					//For Extended S&W - define pipeline window size


//----- Type defines ----------------------------------------------------------
typedef unsigned char		u8b_t;    	// a char
typedef unsigned short		u16b_t;  	// 16-bit word
typedef unsigned int		u32b_t;		// 32-bit word 

extern float LOSS_RATE, ERR_RATE;


/* this function is for simulating packet loss or corruption in an unreliable channel */
/***
Assume we have registered the target peer address with the UDP socket by the connect()
function, udt_send() uses send() function (instead of sendto() function) to send 
a UDP datagram.
***/
int udt_send(int fd, void * pkt, int pktLen, unsigned int flags) {
	double randomNum = 0.0;

	/* simulate packet loss */
	//randomly generate a number between 0 and 1
	randomNum = (double)rand() / RAND_MAX;
	if (randomNum < LOSS_RATE){
		//simulate packet loss of unreliable send
		printf("WARNING: udt_send: Packet lost in unreliable layer!!!!!!\n");
		return pktLen;
	}

	/* simulate packet corruption */
	//randomly generate a number between 0 and 1
	randomNum = (double)rand() / RAND_MAX;
	if (randomNum < ERR_RATE){
		//clone the packet
		u8b_t errmsg[pktLen];
		memcpy(errmsg, pkt, pktLen);
		//change a char of the packet
		int position = rand() % pktLen;
		if (errmsg[position] > 1) errmsg[position] -= 2;
		else errmsg[position] = 254;
		printf("WARNING: udt_send: Packet corrupted in unreliable layer!!!!!!\n");
		return send(fd, errmsg, pktLen, 0);
	} else 	// transmit original packet
		return send(fd, pkt, pktLen, 0);
}

/* this function is for calculating the 16-bit checksum of a message */
/***
Source: UNIX Network Programming, Vol 1 (by W.R. Stevens et. al)
***/
u16b_t checksum(u8b_t *msg, u16b_t bytecount)
{
	u32b_t sum = 0;
	u16b_t * addr = (u16b_t *)msg;
	u16b_t word = 0;
	
	// add 16-bit by 16-bit
	while(bytecount > 1)
	{
		sum += *addr++;
		bytecount -= 2;
	}
	
	// Add left-over byte, if any
	if (bytecount > 0) {
		*(u8b_t *)(&word) = *(u8b_t *)addr;
		sum += word;
	}
	
	// Fold 32-bit sum to 16 bits
	while (sum>>16) 
		sum = (sum & 0xFFFF) + (sum >> 16);
	
	word = ~sum;
	
	return word;
}

//----- Type defines ----------------------------------------------------------

// define your data structures and global variables in here

//packet header
struct pk_header {
   u8b_t type;
   u8b_t seq_number;
   u16b_t checksum;
};

//make packet
u8b_t* make_pkt(char *msg, int length, u8b_t seq_num) {
  u8b_t* packet = (u8b_t*)malloc(sizeof(pk_header)+length);
   (*(pk_header*)packet).type = 0;
   (*(pk_header*)packet).seq_number = seq_num;
   memcpy(packet + sizeof(pk_header), msg, length);
   //   memcpy((void*)((char*)packet + sizeof(pk_header)), (void*)msg, length);
   (*(pk_header*)packet).checksum = 0;
   (*(pk_header*)packet).checksum = checksum(packet, sizeof(pk_header)+length);
   return packet;
}

//mack acknowledgement
void* make_ack(u8b_t seq_num) {
   void* ack = malloc(sizeof(pk_header));
   (*(pk_header*)ack).type = 1;
   (*(pk_header*)ack).seq_number = seq_num;
   (*(pk_header*)ack).checksum = 0;
   (*(pk_header*)ack).checksum = checksum((u8b_t*)ack, sizeof(pk_header));
   return ack;
}

//count number of packets given message length
u8b_t count_pkt(int length) {
  return (length%PAYLOAD==0)?(u8b_t)(length/PAYLOAD):(u8b_t)(length/PAYLOAD+1);
}

//check if a 8-bit number falls between the other two or not
bool falls_between(u8b_t a, u8b_t b, u8b_t c) {
  if (b<c && a>=b && a<= c) return true;
  if (b>c && (a>=b || a<=c)) return true;
  return false;
}

u8b_t seq_num=0;     //current sequence number awaiting for send or ack from sender
u8b_t exp_seq_num=0; //expected sequence number from receiver

int rdt_socket();
int rdt_bind(int fd, u16b_t port);
int rdt_target(int fd, char * peer_name, u16b_t peer_port);
int rdt_send(int fd, char * msg, int length);
int rdt_recv(int fd, char * msg, int length);
int rdt_close(int fd);

/* Application process calls this function to create the RDT socket.
   return	-> the socket descriptor on success, -1 on error 
*/
int rdt_socket() {
//same as part 1
   return socket(AF_INET, SOCK_DGRAM, 0);
}

/* Application process calls this function to specify the IP address
   and port number used by itself and assigns them to the RDT socket.
   return	-> 0 on success, -1 on error
*/
int rdt_bind(int fd, u16b_t port){
//same as part 1
   struct sockaddr_in my_addr;
   my_addr.sin_family = AF_INET;
   my_addr.sin_port = htons(port);
   my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   memset(&(my_addr.sin_zero), 0, 8);
   return bind(fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in));
}

/* Application process calls this function to specify the IP address
   and port number used by remote process and associates them to the 
   RDT socket.
   return	-> 0 on success, -1 on error
*/
int rdt_target(int fd, char * peer_name, u16b_t peer_port){
//same as part 1
   struct hostent *hos = gethostbyname(peer_name);
   struct sockaddr_in peer_addr;
   peer_addr.sin_family = AF_INET;
   peer_addr.sin_port = htons(peer_port);
   peer_addr.sin_addr = *((struct in_addr*)hos->h_addr);
   memset(&(peer_addr.sin_zero), 0, 8);
   return connect(fd, (struct sockaddr*)&peer_addr, sizeof(struct sockaddr_in));
}

/* Application process calls this function to transmit a message to
   target (rdt_target) remote process through RDT socket; this call will
   not return until the whole message has been successfully transmitted
   or when encountered errors.
   msg		-> pointer to the application's send buffer
   length	-> length of application message
   return	-> size of data sent on success, -1 on error
*/
int rdt_send(int fd, char * msg, int length){
//implement the Extended Stop-and-Wait ARQ logic

//must use the udt_send() function to send data via the unreliable layer
   
  //define sequence number variables for cumulative ack
  u8b_t wait_seq_num = seq_num;//seq num that is waiting for ack
  u8b_t recvd_up_to = 0;//num of in-order package already received
  u8b_t base_seq_num = seq_num;

   //prepare and send packets
   u8b_t npkt = count_pkt(length);
   printf("Number of packets: %hhu\n", npkt);
   u8b_t* my_packet = (u8b_t*)malloc(sizeof(char)*length + sizeof(pk_header)*npkt);
   int msg_length[npkt];
   
   for (u8b_t i = 0; i < npkt; ++i) {
     //make packet and send
     msg_length[i] = (i==npkt-1)? length-PAYLOAD*(npkt-1) : PAYLOAD;
     u8b_t* current_pkt = make_pkt(msg, msg_length[i], seq_num);
     memcpy(my_packet + i*(PAYLOAD+sizeof(pk_header)), (u8b_t*)current_pkt, msg_length[i]+sizeof(pk_header));
     udt_send(fd, current_pkt, sizeof(pk_header)+msg_length[i], 0);
     printf("rdt_send: Sent one message of size:%lu seqNo:%hhu\n", sizeof(pk_header)+msg_length[i], seq_num);
     seq_num++;
     if(i==npkt-1) break;
     //shorten the message
     char* temp = (char*)malloc(sizeof(char)*(length-(i+1)*PAYLOAD));
     memcpy(temp, msg+PAYLOAD, length-(i+1)*PAYLOAD);
     msg = temp;
   }

   //preparing for select()
   fd_set read_fds;
   struct timeval timer;
   timer.tv_sec = 0;
   timer.tv_usec = TIMEOUT;

   //preparing for receive buffer for ack
   void* receive_buffer = malloc(sizeof(pk_header)+PAYLOAD);
   int recv_buffer_length;

   //loop - end when received correct ack or with the knowledge that peer has received message correctly (specially for the case during handshake)
   while(1){
	FD_ZERO(&read_fds);
	FD_SET(fd, &read_fds);
	int select_result = select(fd+1, &read_fds, NULL, NULL, &timer);
	if (select_result == -1) {
		perror("select error!\n");
		exit(1);
	}
	//if timeout
	else if (select_result == 0) {
	  for (u8b_t i=recvd_up_to; i<npkt; ++i) {
	    void* current_pkt = (char*)my_packet + i*(PAYLOAD+sizeof(pk_header));
	    udt_send(fd, current_pkt, sizeof(pk_header)+msg_length[i], 0);
	    printf("rdt_send: Retransmit one message of size:%lu seqNo:%hhu\n", sizeof(pk_header)+msg_length[i], (*(pk_header*)current_pkt).seq_number);
	  }
	  timer.tv_sec = 0;
	  timer.tv_usec = TIMEOUT;
	}
	//if no error or timeout
	else {
	  if (FD_ISSET(fd, &read_fds)) {
	    recv_buffer_length = recv(fd, receive_buffer, sizeof(pk_header)+PAYLOAD, 0);
	    u8b_t recv_seq_num = (*(pk_header*)receive_buffer).seq_number;
	    //if ack
	    if ((*(pk_header*)receive_buffer).type==1) {
	      //if ack correct
	      if (checksum((u8b_t*)receive_buffer, recv_buffer_length)==0) {
		printf("rdt_send: Received the ACK:%hhu (%hhu-%d)\n", recv_seq_num, base_seq_num, (seq_num==0)?255:seq_num-1);
		if (recv_seq_num == ((seq_num==0)?255:seq_num-1)) {
		  break;
		} else if (falls_between(recv_seq_num, wait_seq_num, (seq_num>=2)?seq_num-2:(254+seq_num))) {
		  recvd_up_to += (1+recv_seq_num - wait_seq_num); 
		  wait_seq_num = recv_seq_num+1;
		}
	      }
	    }
	    //if data
	    else {
	      //if data not corrupted
	      if (checksum((u8b_t*)receive_buffer, recv_buffer_length)==0) {
		if (wait_seq_num==0&&recv_seq_num<2) {
		  void* my_ack = make_ack(recv_seq_num);
		  udt_send(fd, my_ack, sizeof(pk_header), 0);
		} else
		  break;
	      }
	    }
	  }
	}
   }//end while
   return length;

}

/* Application process calls this function to wait for a message of any
   length from the remote process; the caller will be blocked waiting for
   the arrival of the message. 
   msg		-> pointer to the receiving buffer
   length	-> length of receiving buffer
   return	-> size of data received on success, -1 on error
*/
int rdt_recv(int fd, char * msg, int length){
//implement the Extended Stop-and-Wait ARQ logic
  void* receive_buffer = malloc(sizeof(pk_header)+PAYLOAD);
  int receive_result;
  while(1) {
    receive_result = recv(fd, receive_buffer, sizeof(pk_header)+length, 0);
    if (receive_result==-1) {
      perror("error in receiving!\n");
      exit(1);
    } else {// received packet
      //if data
      if ((*(pk_header*)receive_buffer).type==0) {
	//if data correct
	if (exp_seq_num == (*(pk_header*)receive_buffer).seq_number && checksum((u8b_t*)receive_buffer, receive_result)==0) {
	  printf("rdt_recv: Got an expected packet - seqNo: %hhu\n", exp_seq_num);
	  //save data to *msg
	  memcpy(msg, ((char*)receive_buffer + sizeof(pk_header)), length);
	  free(receive_buffer);
	  //send ack
	  void* my_ack = make_ack(exp_seq_num);
	  udt_send(fd, my_ack, sizeof(pk_header), 0);
	  //update expected sequence number
	  exp_seq_num = exp_seq_num + 1;
	  break;
	}
	//if data corrupted or with wrong seq number
	else {
	  printf("Got out-of-order or corrupted packet\nDrop the packet\nRetransmit the ACK packet\n");
	  void* my_ack = make_ack((exp_seq_num==0)?255:exp_seq_num-1);
	  udt_send(fd, my_ack, sizeof(pk_header), 0);
	}
      }//end if
      //if ack
      else {
	continue;
      }
    }//end if
  }//end while
  return receive_result - sizeof(pk_header);

}

/* Application process calls this function to close the RDT socket.
*/
int rdt_close(int fd){
//implement the Extended Stop-and-Wait ARQ logic
  //preparing for select()
  struct timeval timer;
  timer.tv_sec = 0;
  timer.tv_usec = TWAIT;
  fd_set read_fds;
  void* receive_buffer = malloc(sizeof(pk_header)+PAYLOAD);
  int recv_buffer_length;

  //loop until time-wait expires
  while(1) {
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);
    int select_result = select(fd+1, &read_fds, NULL, NULL, &timer);
    if (select_result == -1) {
      perror("select error!\n");
      exit(1);
    }
    //if timeout
    else if (select_result == 0) {
      free(receive_buffer);
      break;
    }
    //if retransmitted packet received
    else {
      if (FD_ISSET(fd, &read_fds)) {
	recv_buffer_length = recv(fd, receive_buffer, sizeof(pk_header)+PAYLOAD, 0);
	//if data correct
	if ((*(pk_header*)receive_buffer).type == 0 && checksum((u8b_t*)receive_buffer, recv_buffer_length)==0) {
	  void* my_ack = make_ack((*(pk_header*)receive_buffer).seq_number);
	  udt_send(fd, my_ack, sizeof(pk_header), 0);
	  timer.tv_sec = 0;
	  timer.tv_usec = TWAIT;
	}
      }
    }
  }
  return close(fd);

}

#endif
