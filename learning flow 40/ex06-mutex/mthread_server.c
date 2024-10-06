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
#define MAX_CLIENTS 10

pthread_mutex_t mutex_b = PTHREAD_MUTEX_INITIALIZER;
char buffer_client[BUFFER_SIZE] = {0};
int active_clients = 0;
int sockfd;
int i;
int flag;

typedef struct{
    struct sockaddr_in clientaddr;
    char* current_text;
    pthread_mutex_t mutex_s;
}client_info;


void* deal_with_client(void* arg) {

    printf("starting thread\n");
    //check again if the max nb of client has been reached
    if (active_clients>=MAX_CLIENTS) {
        pthread_exit(NULL);
	}

    //get client's info
    client_info* c_i = (client_info*)arg;

    printf("[server] Dealing with client: %s:%d\n", inet_ntoa(c_i->clientaddr.sin_addr),ntohs(c_i->clientaddr.sin_port));

    //infinite loop until clients stops sending
    while (1) {
        pthread_mutex_lock(&(c_i->mutex_s)); //start critical region
        // Send the text back to the client
        int s = sendto(sockfd, c_i->current_text, strlen(c_i->current_text), 0,
                     (struct sockaddr*)&(c_i->clientaddr), sizeof(struct sockaddr));
        if (s < 0) {
            perror("[server] Error: Cannot send text back\n");
            break;
        }
        pthread_mutex_unlock(&(c_i->mutex_s));
        sleep(1);
    }

    printf("Client not sending anymore(adress, port:) %s:%d\n", inet_ntoa(c_i->clientaddr.sin_addr), ntohs(c_i->clientaddr.sin_port));
    free(c_i->current_text);
    pthread_mutex_destroy(&(c_i->mutex_s));
    pthread_exit(NULL);
}

int main(int argc, char** argv) {

    /* check the arguments and load them */
    if (argc < 2) {
        printf("\n[server] Error: Incorrect number of arguments, please enter port number\n");
        return -1;
    }
    int PORT_NUMBER = atoi(argv[1]);  // converts string into int

    client_info* data = (client_info*)calloc(MAX_CLIENTS , sizeof(client_info));
    if (data == NULL) {
        printf("\n[server] Error: Failed to allocate memory to data array\n");
        return 1;
    }

    /* set the server parameters */
    struct sockaddr_in servaddr;
    memset(&servaddr, '\0', sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_NUMBER);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("[server] Error: Cannot create socket\n");
        return -1;
    }

    /* bind the socket to the server address:listen for incoming packages */
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("[server] Error: Cannot bind\n");
        return -1;
    }

    struct sockaddr_in new_clientaddr;
    socklen_t size = sizeof(new_clientaddr);

    //continuously listens for client requests
    while (1) {

        printf("[server] Server listening on port %d\n", PORT_NUMBER);

        // Receive client connection request
        int len = recvfrom(sockfd, buffer_client, BUFFER_SIZE , 0, (struct sockaddr*)&new_clientaddr, &size);
        if (len < 0) {
            perror("[server] Error: Cannot receive connection request\n");
            break;
        }

        //check if the maximum number of clients has been reached
        if (active_clients >= MAX_CLIENTS) {
            printf("[server] Error: Maximum number of clients reached, rejecting new connection request.\n");
            continue;
        }

        // check if it is an old client:
        flag=0;
        for ( i = 0; i < active_clients; i++) {
            if (memcmp(&new_clientaddr, &(data[i].clientaddr), size)==0) { //not a new client
                pthread_mutex_lock(&(data[i].mutex_s));
                flag=1;
                printf("not a new client");
                strcpy(data[i].current_text,buffer_client);
                pthread_mutex_unlock(&(data[i].mutex_s));
                break;
                }
            }    
           
        
        if (flag==0 && active_clients<MAX_CLIENTS) { //new client 

            pthread_mutex_lock(&mutex_b);
            data[active_clients].clientaddr = new_clientaddr;
            data[active_clients].current_text = (char*)calloc(BUFFER_SIZE, sizeof(char));
            strcpy(data[active_clients].current_text , buffer_client);
            printf("new client storing message : %s", data[active_clients].current_text);
            pthread_mutex_unlock(&mutex_b);

            pthread_t client_thread;
            pthread_mutex_t mutex_s = PTHREAD_MUTEX_INITIALIZER;
            data[active_clients].mutex_s=mutex_s;
            if (pthread_create(&client_thread, NULL, deal_with_client, &(data[active_clients])) != 0) {
                perror("[server] Error: Cannot create client thread\n");
                return -1;
            }
            active_clients++;
        }


    }//end while loop
        

    free(data);
    close(sockfd);

    return 0;
}
