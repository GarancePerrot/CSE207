#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUFFER_SIZE 500 
#define FYI 0x01
#define MYM 0x02
#define END 0x03
#define TXT 0x04
#define MOV 0x05
#define LFT 0x06



void display_bytes(char* array, int array_len) { //helper function to visualize the bytes
    int i;
	for (i=0; i<array_len; i++) {
		printf("%d ",(int)array[i]); //casting the char into int
	}
	printf("\n");
}


int main(int argc, char **argv){

    /*check the arguments and load them*/
    if (argc < 3) {
        printf("\n[client] Incorrect number of arguments, please enter IP address and port number\n");
        return -1;
    }
    char* IP_ADDRESS=argv[1]; 
    int PORT_NUMBER=atoi(argv[2]); //converts string into int

    /*create the socket*/
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		perror("\n[client] Cannot create socket\n");
	    return -1;
	}


    /*set the destination parameters*/
    struct sockaddr_in servaddr; 
    memset(&servaddr, '\0' , sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port = htons(PORT_NUMBER);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS); //inet_addr converts the char into a valid ip address


    /* send message "Hello!" to server*/
    char hello[7] = "Hello!\n";

    int len = sendto(sockfd,(const char*)hello, 7, 0,
        (const struct sockaddr *)&servaddr, sizeof(servaddr)); 
    if (len < 0) {
        perror("\n[client] Cannot send hello\n");
        return -1;
    } 
    printf("\n[client] Sent data:%s\n", hello); 


    /*initialize the playing value and create the listening tools and the board*/
    int playing=1;
    char recv_msg[BUFFER_SIZE] ={0};
    char type;
    char game[3][3] = {
    {' ', ' ', ' '},
    {' ', ' ', ' '},
    {' ', ' ', ' '}
    };
    int col,row;
    char mov[3]={0};


    
    /*start the loop for listening server's messages*/
    while (playing) {
 
        socklen_t serversize = sizeof(servaddr);
        int r = recvfrom(sockfd,(char *)recv_msg,BUFFER_SIZE,0, 
            (struct sockaddr *)&servaddr, &serversize);
        if (r == 0) {
            break;
        }
        else if (r < 0){
            perror("\n[client] Cannot receive data\n");
            close(sockfd);
            return -1;
        }

        recv_msg[r] = '\0';
        //printf("\n[client] Received server's message: %s\n", recv_msg);
        //display_bytes(recv_msg, sizeof(recv_msg));


        /*check the type of message and acting accordingly*/
        type = recv_msg[0];

        if (type == TXT) { //display the content on screen
            printf("\n[client] Displaying TXT from server: %s\n", recv_msg);
        }//end TXT

        else if (type==END) {
            int winner = recv_msg[1];
            if ((winner==1) || (winner==2)){ //identify the winner and print it
                printf("\n[client]The winner is player %d!\n", winner);
            }
            else if (winner==0) { //in case of a draw
                printf("\n[client] No winner, the game is a draw :(\n");
            }
            else {
                printf("\n[client] Already two players playing, client rejected :(\n");
            }
            playing = 0;
        }//end END

        else if (type==MYM) { //make your move

            /*ask the user for a position on the board and send it to server*/

            printf("\n[client] Enter 'row column' each between 0 and 2 , press Enter when finished : \n");

            
            scanf("%d %d", &row,&col);
            printf("\n[client] Your position :%d %d\n",row,col );
            //char mov[3]={0};
            mov[0] = (char)MOV;
            mov[1] = (char)col;
            mov[2]= (char)row;

            serversize = sizeof(servaddr);
            int s = sendto(sockfd,mov, 3, 0, (const struct sockaddr*)&servaddr, serversize);
            if (s == -1) {
                perror("\n[client] Cannot send MOV message\n");
                return -1;
            }

        }//end MYM

        else if (type==FYI) { //For Your Information
            /* record the state of the board and display it in the screen for the user*/
            int n = recv_msg[1]; //nb of positions filled
            printf("\n[client] %d filled positions. Here is the board's current state:\n", n);
            int i;
            for (i=2 ; i<2+3*n ; i+=3){ //iterate over the recv_msg three by three starting at the first player id

                //identify the player
                char play_i;
                if (recv_msg[i]==1) {
                    play_i = 'X';
                }
                else {
                    play_i = 'O';
                }

                //fill in the game board
                int col_i = (int)recv_msg[i+1];
                int row_i = (int)recv_msg[i+2];
                game[row_i][col_i] = play_i;
            }
                /*
                print it for the user using nested loops:
                   k=0 k=1 k=2
                j=0   |   |   \n
                    - + - + - \n
                j=1   |   |   \n
                    - + - + - \n
                j=2   |   |   \n   
                */
                int j , k;
                for (j=0; j<3; j++) {
                    for (k=0; k<3; k++) {
                        printf("%c ", game[j][k]);
                        if (k<2) {
                            printf(" |");
                        }
                    }
                    printf("\n");
                    if (j<2) {
                        printf(" - + - + - \n");
                    }
                }

        }//end FYI

    } //end playing loop

    /*close the socket*/
    close(sockfd);
    return 0;
}