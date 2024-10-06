#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUFFER_SIZE 100 

int main(int argc, char **argv){

    /*check the arguments and load them*/
    if (argc < 2) {
        printf("\nIncorrect number of arguments, please enter IP address and port number\n");
        return -1;
    }
    char* IP_ADDRESS=argv[1]; 
    int PORT_NUMBER=atoi(argv[2]); //converts string into int

    /*create the socket*/
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		perror("Cannot create socket\n");
	    return -1;
	}

    /*set the destination parameters*/
    struct sockaddr_in servaddr; 
    memset(&servaddr, '\0' , sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port = htons(PORT_NUMBER);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS); //inet_addr converts the char into a valid ip address

    /*for lines 36 to 54 I got help from a friend Elie */
    /* get the input from the keyboard making sure to get all the string until newline*/
    char buffer[BUFFER_SIZE] ={0};
    printf("\n[client] Enter data to send, pressing Enter when finished : \n");
    while ((fgets(buffer, BUFFER_SIZE, stdin) != NULL) && (buffer[0] !='\n') ) {
        
        if ((strlen(buffer) == BUFFER_SIZE-1) && (buffer[BUFFER_SIZE-1] !='\n')){
            perror("Input line is too long\n");
            return -1;
        } 

        /*send the data packets*/
        int len = sendto(sockfd, buffer, BUFFER_SIZE, 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr)); 
        printf("\n[client] Sent data:%s", buffer); 
        if (len < 0) {
		    perror("Cannot send packages\n");
        } 
    }

        /*close the socket*/
    close(sockfd);
    return 0;
}