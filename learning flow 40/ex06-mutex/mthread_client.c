#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#define BUFFER_SIZE 100 


struct server_info{
    int sockfd;
    struct sockaddr_in servaddr;
};

void* read_user(void* arg){
    struct server_info* s_i = (struct server_info*)arg;
    int sockfd = s_i->sockfd;
    struct sockaddr_in servaddr = s_i->servaddr;



    /* get the input from the keyboard making sure to get all the string until newline*/
    char buffer_user[BUFFER_SIZE] ={0};
    printf("\n[client] Enter data to send, pressing Enter when finished : \n");
    while ((fgets(buffer_user, BUFFER_SIZE, stdin) != NULL) && (buffer_user[0] !='\n') ) {
        
        if ((strlen(buffer_user) == BUFFER_SIZE-1) && (buffer_user[BUFFER_SIZE-1] !='\n')){
            perror("Input line is too long\n");
        } 

        /*send the data packets*/
        int len = sendto(sockfd, buffer_user, BUFFER_SIZE, 0,
           (struct sockaddr *)&servaddr, sizeof(servaddr)); 
        printf("\n[client] Sent data:%s", buffer_user); 
        if (len < 0) {
		    perror("Cannot send packages\n");
        } 
    }

    pthread_exit(NULL);

}


void* rcv_server(void* arg){
    struct server_info* s_i = (struct server_info*)arg;
    int sockfd = s_i->sockfd;
    struct sockaddr_in servaddr = s_i->servaddr;
    char buffer_server[BUFFER_SIZE] = {0};

    while(1){

        // Receive the server's reply
        socklen_t serversize = sizeof(servaddr);
        int len = recvfrom(sockfd, buffer_server, BUFFER_SIZE, 0, (struct sockaddr*)&servaddr, &serversize);
        if (len < 0) {
            perror("Cannot receive reply\n");
            break;
        }

        // Show the received reply
        printf("\n[client] Received server's answer: %s\n", buffer_server);

        // Release the mutex lock

    }
    pthread_exit(NULL);

}


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


    struct server_info s_i;
    s_i.sockfd = sockfd;
    s_i.servaddr = servaddr;

    pthread_t read_user_thread;
    if (pthread_create(&read_user_thread, NULL, read_user, &s_i) != 0) {
        perror("Failed to create read-user thread\n");
        return -1;
    }

    pthread_t rcv_server_thread;
    if (pthread_create(&rcv_server_thread, NULL, rcv_server, &s_i) != 0) {
        perror("Failed to create receive-server thread\n");
        return -1;
    }

    if(pthread_join(read_user_thread,NULL)){
		fprintf(stderr, "Error joining threads\n");
		return 1;
    };

    if(pthread_join(rcv_server_thread,NULL)){
		fprintf(stderr, "Error joining threads\n");
		return 1;
    };

    /*close the socket*/
    close(sockfd);
    return 0;
}