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
    if (argc < 1) {
        printf("\nIncorrect number of arguments, please enter the port number\n");
        return -1;
    } 
    int PORT_NUMBER=atoi(argv[1]); //converts string into int

    /*create the socket*/
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		perror("Cannot create socket\n");
	    return -1;
	}


    /*set the client and destination parameters*/
    struct sockaddr_in clientaddr , servaddr;//create a client struct

    memset(&servaddr, '\0', sizeof(servaddr)); 
    servaddr.sin_family=AF_INET;
    servaddr.sin_port = htons(PORT_NUMBER);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    /*bind the socket to the port = listen the incoming packets, reserve the port*/
    int b = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (b < 0) {
		perror("Cannot bind to port\n");
        close(sockfd);
        return -1;
	}

    /*receive the data*/
    char recv_data[BUFFER_SIZE] = {0};
    socklen_t clientsize = sizeof(clientaddr);

    int r = recvfrom(sockfd, recv_data, BUFFER_SIZE, 0,
        (struct sockaddr *)&clientaddr, &clientsize); 
    if (r < 0){
        perror("Cannot receive data\n");
        close(sockfd);
        return -1;
    }
    printf("\n[server] Received client's data:%s", recv_data);

    int len = sendto(sockfd, recv_data, BUFFER_SIZE, 0,
           (struct sockaddr *)&clientaddr, sizeof(clientaddr)); 
    if (len < 0) {
		perror("Cannot send packages\n");
    } //do not exit to make sure to close the socket
    printf("\n[server] Sent back to client:%s", recv_data);

    /*close the socket*/
    close(sockfd);

    return 0;
}