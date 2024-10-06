GARANCE PERROT BX24

I worked on this assignment alone (however, Bogdana Kolic deserves a special mention
 because she helped me debug my codes when I was stuck.)

To test the codes:

> make
Open three terminals T1,T2,T3
T1> ./server 5000 (on any valid port number)
T2> ./client "127.0.0.1" 5000
T3> ./client "127.0.0.1" 5000
The game starts, the first client starts playing with symbol 'X' and the other one with 'O'. 
Open other terminals and try to connect new clients, they are rejected. 
One at a time, the players enter their position under the format 'row column' (without quotes). 
The server updates the board and the latter is displayed for the next player.
The game ends when a player has aligned three of his symbols either in a diagonal, row or column,
or if all the positions are filled (the socket is closed). 

The client sends a greeting message to the server and then listens continuously to it for messages.
It checks the type of message received and acts accordingly:
-TXT: display the message
-END: end the game 
-MYM: ask for the user to type the position on the keyboard and send it to the server
-FYI: update the state of the board and display it on the screen for the user.

The server waits for two clients to connect and then starts a game, refusing any other client request. 
To the client playing, it sends a FYI message with the board info and then asks for a move. 
It then checks if the position is valid: if not, requests another move, otherwise updates the grid
with the position. The server checks the game status and ends the game if necessary, or continues,
switching of player. 

For more coding-related information, both files server.c and client.c contain detailed comments.

