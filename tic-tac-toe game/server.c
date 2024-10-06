#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUFFER_SIZE 200
#define FYI 0x01
#define MYM 0x02
#define END 0x03
#define TXT 0x04
#define MOV 0x05
#define LFT 0x06

typedef struct player{
	struct sockaddr_in clientaddr;
	socklen_t len;
    char symbol;
    int nb_m_row;
    int nb_m_col;
    int nb_m_diag_l; //left diagonal: starts from left bottom to right top
    int nb_m_diag_r; //right diagonal: starts from right bottom to left top
} player;

typedef struct cell{
    player p;
    char col;
    char row;
} cell;

int p_p;
player* players_array={0};


//check if the position is valid (in the board and not already taken) : 0 if it is valid, 1 if not
int is_valid(int col, int row, cell* board, int nb_pos){

    if (col < 0 || row < 0 || col > 2 || row > 2){//not in the board
        return 1; 
    }
    //check for all the players if they have taken the position
    int i;
    for (i = 0; i < nb_pos; i++){
        if (board[i].row == row && board[i].col == col){
            return 1;
        }
    }
    return 0;
}

//check the game status : 0 if it ends with a draw, 1 or 2 for the winner and 3 if it is not the end yet
int check_status(cell* board, char symb,char col, char row, int nb_pos){
    int i;
    if (nb_pos <2){//there can be no winner
        return 3;
    }
    int match;
    for (i=0;i<9;i++) {//iterating over each cell of the board to check the player by its symbol and check position
        if (symb == board[i].p.symbol){ //same symbol => same player
            if ((board[i].col == col)){ //if on the same column
                if (board[i].row != row ){ //not count itself
                    players_array[p_p-1].nb_m_col+=1;
                    //printf("player : %d ,  nb_m_col: %d", p_p , players_array[p_p-1].nb_m_col);
                }
            }
            else if ((board[i].row == row)){ //if on the same row
                if (board[i].col != col ){ //not count itself
                    players_array[p_p-1].nb_m_row+=1;
                    //printf("player : %d ,  nb_m_row: %d", p_p , players_array[p_p-1].nb_m_row);
                }
            }
            else if (((int)row + (int)col)%3 == 2 ){ //if the position is on the left diagonal
                if ((int)(board[i].row) + (int)(board[i].col)%3 == 2){ //if on the same diagonal
                    players_array[p_p-1].nb_m_diag_l+=1;
                    //printf("player : %d ,  nb_m_diag_l: %d", p_p , players_array[p_p-1].nb_m_diag_l);
                }
            }
            else if (row == col){ //if the position is on the right diagonal
                if (board[i].row == board[i].col){ //if on the same diagonal
                    players_array[p_p-1].nb_m_diag_r+=1;
                    //printf("player : %d ,  nb_m_diag_r: %d", p_p , players_array[p_p-1].nb_m_diag_r);
                }
            }
        }//end if same symbol
        if (players_array[p_p-1].nb_m_row==2 || players_array[p_p-1].nb_m_col==2 || players_array[p_p-1].nb_m_diag_l==2 || players_array[p_p-1].nb_m_diag_r==2){ //have found a match, no need to continue
            match=1;
            //printf("found match");
            break;
        }
    }//end starting over the board

    if (match==1){ //we have found a match, return the winner
        if (symb == 'X'){ 
            return 1;
        }
        else if (symb== 'O'){
            return 2;
        }
    }
    if (nb_pos == 8){//in case of a draw
        return 0;
    }
    return 3; //the game is not finished
}



int main(int argc, char **argv){
    
    int i;//for iterations

    /*check the arguments and load them*/
    if (argc < 2) {
        printf("\n[server]Incorrect number of arguments, please enter the port number\n");
        return -1;
    } 
    int PORT_NUMBER=atoi(argv[1]); //converts string into int

    /*create the socket*/
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
		perror("[server]Cannot create socket\n");
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
		perror("[server]Cannot bind to port\n");
        close(sockfd);
        return -1;
	}
    printf("[server]Waiting for clients on port %d \n", PORT_NUMBER);

    /*tools to receive the data and accept connection of two players:*/
    int nb_players=0;
    char recv_data[BUFFER_SIZE] = {0};
    //socklen_t clientsize = sizeof(clientaddr);
    players_array = (player*)calloc(2, sizeof(player));
    players_array[0].symbol = 'X';//player 1 plays with 'X'
    players_array[1].symbol = 'O';//player 2 plays with 'O'
    players_array[0].nb_m_col =0;
    players_array[0].nb_m_diag_l=0;
    players_array[0].nb_m_diag_r=0;
    players_array[0].nb_m_row=0;
    players_array[1].nb_m_col =0;
    players_array[1].nb_m_diag_l=0;
    players_array[1].nb_m_diag_r=0;
    players_array[1].nb_m_row=0;


    p_p=1; //player_playing, the first one starts

    while (nb_players<2){
        socklen_t clientsize = sizeof(clientaddr);
        int r = recvfrom(sockfd, recv_data, BUFFER_SIZE, 0,
            (struct sockaddr *)&clientaddr, &clientsize); 
        if (r < 0){
            perror("[server]Cannot receive data\n");
            close(sockfd);
            return -1;
        }
        printf("\n[server] Received client's data:%s", recv_data);

        //store the info for players and assign their symbol
        nb_players+=1;
        players_array[nb_players-1].clientaddr = clientaddr;
        players_array[nb_players-1].len  = clientsize;
        printf("[server] Welcome player %d ! \n", nb_players);

    }//end while loop: two players have connected


    /*initialize tools for the game*/
    int nb_pos = 0; //nb of positions filled
    char recv_msg[BUFFER_SIZE];
    int playing_game=1; //0 or 1
    int request_move; //0 or 1
    struct sockaddr_in client_playing;
    socklen_t client_len;
    cell* board = (cell*)calloc(9, sizeof(cell)); //create the game board with 9 positions 
    //for (i = 0; i < 9; i++) {
    //    board[i].p.symbol = malloc(sizeof(char)); // allocate memory for the symbol in each player object
    //}
    int col, row;


    while(playing_game){
        memset(recv_msg, 0, BUFFER_SIZE); //reinitialize the buffer
        client_playing = players_array[p_p-1].clientaddr;
        client_len = players_array[p_p-1].len;

        /*create and send FYI message */
        int size_msg = 2*sizeof(char) + 3*nb_pos*sizeof(char);
        char* send_msg = malloc(size_msg);
        char* ptr = send_msg;
        *ptr = (char)FYI;
        ptr++;
        *ptr = (char)nb_pos;
        ptr++;
        for (i = 0; i < nb_pos; i++) {
            if (board[i].p.symbol == 'X'){
                *ptr = 1;
            }
            else {
                *ptr = 2;
            }
            memcpy(ptr+1, &(board[i].col), 1);
            memcpy(ptr+2, &(board[i].row),1);
            ptr += 3; 
	    }
        
        int s = sendto(sockfd, send_msg, size_msg, 0, (struct sockaddr *)&client_playing, client_len);
        if (s<0) {
            perror("\n[server] Cannot send FYI message\n");
            return -1;
        } 
        free(send_msg);

        /*requesting the player's move until it is valid*/
        request_move = 1;
        while(request_move){
            
            /*create and send MYM message*/
            char mym[1] ={0};
            mym[0] = (char)MYM;
            int s = sendto(sockfd, mym, 1, 0, (struct sockaddr *)&client_playing, client_len);
            if (s<0) {
                perror("\n[server] Cannot send MYM message\n");
                return -1;
            } 

            struct sockaddr_in client_unknown;
            socklen_t c_u_size = sizeof(client_unknown);
            /*receive someone's message and check if it is the current player's move*/
            while (1){
                int r = recvfrom(sockfd, recv_msg, BUFFER_SIZE, 0, ( struct sockaddr *)&client_unknown, &c_u_size);
                if (r<0) {
                    perror("\n[server] Cannot receive data\n");
                    return -1;
                } 
                if( memcmp((struct sockaddr_in*)&client_unknown,(struct sockaddr_in*)&client_playing,client_len) !=0){ //not current player sending
                    /*send END message with value 255*/
                    printf("sending no the 3rd client");
                    char end[2]={0};
                    end[0]= (char)END;
                    end[1] = (char)0xFF;
                    int s = sendto(sockfd, end, 2, 0, (struct sockaddr *)&client_unknown, c_u_size);
                    if (s<0) {
                        perror("\n[server] Cannot send END message to additional client\n");
                        return -1;
                    } 
                }
                else { //the current player has sent his mov message
                    break;
                }
            }
            /*check if the position chosen by the player is valid*/
            col = (int)recv_msg[1];
            row = (int)recv_msg[2];
            request_move = is_valid(col,row, board, nb_pos);//0 if valid, 1 if invalid

            if (request_move ==1){
                /*send TXT message saying invalid move*/
                char text[13] = "Invalid move\0";
                char txt[14] ={0};
                txt[0] = (char)TXT;
                char* ptr = txt+1;
                strcpy(ptr, text);
                int s = sendto(sockfd,txt, 14, 0, (struct sockaddr *)&client_playing, client_len);
                if (s<0) {
                    perror("\n[server] Cannot send TXT message\n");
                    return -1;
                } 
            }
        }//end requesting move (request_move = 0 when getting out of loop)

        /*fill the grid with the position*/
        board[nb_pos].p = players_array[p_p-1];
        board[nb_pos].col = (char)col;
        board[nb_pos].row = (char)row;
        printf("[server]Position col=%d, row=%d filled in board\n", col, row);

        /*check the game status*/
        char symb = players_array[p_p-1].symbol;
        int status = check_status(board, symb, (char)col, (char)row ,nb_pos);

        /*if the game is finished, send the END message with the winner and terminate the game*/
        if (status != 3){ 
            char end[2] = {0};
            end[0] = (char)END;
            end[1] = (char)status;
            int s = sendto(sockfd, end, 2, 0, (struct sockaddr *)&(players_array[0].clientaddr), client_len);
            if (s<0) {
                perror("\n[server] Cannot send END message\n");
                return -1;
            } 
            s = sendto(sockfd, end, 2, 0, (struct sockaddr *)&(players_array[1].clientaddr), client_len);
            if (s<0) {
                perror("\n[server] Cannot send END message\n");
                return -1;
            } 
            playing_game = 0;
        }

        /*increment nb_positions and change player_playing index*/
        nb_pos+=1;
        if (p_p==1){
            p_p=2;
        }
        else {
            p_p=1;
        }
        //printf("p_p=%d", p_p);
    }//end while playing_game
    
    free(players_array);
    free(board);
    /*close the socket*/
    close(sockfd);
    return 0;
    }
