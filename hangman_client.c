#include <arpa/inet.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <ctype.h>
#include <fcntl.h> 
#include <unistd.h>
#include <string.h>    
#include <sys/socket.h> 

//checks the guess of the user
int guessChecker(char g, char trivia[]);
//checks if the user won the game
int winGame(char trivia[]);
//to check if a guess has already been guessed
int guessedAlready(char g, char wrongGuess[]);

int main(int argc, char* argv[]){
        struct sockaddr_in serv;

        int s; //socket
        //using bzero to clear all the buffers
        char mssg[1000];
        bzero(mssg,1000);
        char servReply[2000];
        bzero(servReply,2000);
        char guess[50];
        bzero(guess,50);
        char wrongGuess[50];
        memset(wrongGuess, '\0', sizeof(wrongGuess));
        bzero(wrongGuess, 50);
        char trivia [50] ={'\0'};
        bzero(trivia,50);

        //Create socket
        s = socket(AF_INET , SOCK_STREAM , 0);
        if (s == -1) {
                printf("Error: Unable to create socket\n");
        }
        printf("Socket created\n");

        serv.sin_addr.s_addr = inet_addr(argv[1]);
        serv.sin_family = AF_INET;
        serv.sin_port = htons(atoi(argv[2]));

        //Connecting to server
        if (connect(s , (struct sockaddr *)&serv , sizeof(serv)) < 0) {
                printf("Error: Connecting failed\n");
                return 1;
        }
        
        //checking if we received anything
        if( recv(s , servReply , 2000 , 0) < 0){
                puts("Error: Receive failed\n");
        }
        
        printf("%s\n",servReply);

        //checking if we are within first 3 and able to connect to play the game
        if(strcmp(servReply,"Error: Server is full\n") == 0){
                close(s);
                return 0;
        }

        //getting input
        fgets(servReply, sizeof(servReply), stdin);

        //taking out the newline
        if (servReply[strlen(servReply) - 1] == '\n'){
        servReply[strlen(servReply) - 1] = '\0';
        }
        
        //sending reply
        send(s, servReply, strlen(servReply), stdin);

        //checking if user wants to play
        if(strcmp(servReply,"y") == 0){
                
                //keeping count of wrong guesses
                int count = 7;

                while(count > 0 ) {
                        //making sure we received something
                        if( recv(s , trivia , 2000 , 0) < 0){
                                puts("Error: Receive failed\n");
                        }
                        //checking if we have won the game
                        if(winGame(trivia)) {
                                break;
                        }
                        //checking if the guess is wrong and adding
                        //the wrong guess to list of guesses
                        if(!guessChecker(guess[0],trivia)){
                                strcat(wrongGuess,guess);
                                count--;
                        }
                        
                        printf("%s\n",trivia);
                        printf("Incorrect Guesses: ");
                        printf("%s\n\n",wrongGuess);

                        printf("Letter to Guess: ");

                        fgets(guess, sizeof(servReply), stdin);
                        if (guess[strlen(guess) - 1] == '\n'){
                                guess[strlen(guess) - 1] = '\0';
                        }

                        int wrong = 0;

                        while(wrong == 0){
                                //checking if we have right input
                                if((!guessedAlready(guess[0], wrongGuess)) && ( strlen(guess)==1) && ((guess[0]>=65 && guess[0]<=90) || (guess[0]>=97 && guess[0]<=122)) ){
                                        if( send(s , guess , strlen(guess) , 0) < 0){
                                                printf("Send failed\n");
                                                return 1;
                                        }
                                        wrong = 1;
                                //asking the user again to make sure we get a right input
                                }else{
                                        printf("Error! Please guess one letter.\n");
                                        printf("Letter to Guess: ");
                                        fgets(guess, sizeof(guess), stdin);
                                        if (guess[strlen(guess) - 1] == '\n'){
                                                guess[strlen(guess) - 1] = '\0';
                                        }

                                        //making sure eeverything is lowerspace
                                        for(int i=0; i<strlen(guess); i++){
                                            guess[i]=tolower(guess[i]);
                                        }
                                }
                        }
                }
                //clearing guess
                bzero(guess,50);
                recv(s, guess, 2000, 0);
                //checking if we won
                if(winGame(trivia)) {
                        printf("The Word: %s\n",trivia);
                        printf("You Win\nGame Over!\n");
                }else if(count==0) {
                        printf("You Lose\nGame Over!\n");
                        strcpy(guess,"end");

                        if( send(s , guess , strlen(guess) , 0) < 0){
                                printf("Error: Send failed\n");
                                return 1;
                        }
    
                        if( recv(s , guess , 2000 , 0) < 0){
                                puts("Error: Receive failed\n");
                        }
    
                        printf("Correct Word was: %s",guess);
                }
        }
        
        close(s);
        return 0;
}
    
int guessChecker(char g, char trivia[]) {
        int check=0;
    
        for(int i=0; i<strlen(trivia); i++) {
                if(trivia[i] == g)
                         check=1;
        }
    
        return check;
}

int guessedAlready(char g, char wrongGuess[]) {
        int check = 0;

        for(int i = 0; i<strlen(wrongGuess); i++) {
                if(wrongGuess[i] == g)
                        check =1;
        }

        return check;
}

int winGame(char trivia[]) {
        int check=1;
    
        for(int i=0; i<strlen(trivia); i++) {
                if(trivia[i] == '_') {
                        check=0;
                }
        }

        return check;
}
    
