#include <arpa/inet.h> 
#include <pthread.h>
#include <fcntl.h> 
#include <time.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>   

//defining the threads
#define NUM_THREADS 5
//checking to see if the guess is right 
void guessChecker(char g, char w[], char trivia[]);
//getting thread id
void *printThreadID(void *id);
//getting a random num to chose the word
int randomNum();
//checking to see if won game
int winGame(char trivia[]);

char * words[15];

int n_threads;

int main(int argc, char* argv[]) {
        int socket_desc, client_sock, c , read_size;
        struct sockaddr_in server , client;
        char client_message[2000];
        char t[5];

        FILE* file;
        file = fopen("hangman.txt", "r");
        char* line = NULL;
        size_t length = 0;
        int wordCount = 0;
        ssize_t read;
        if(file) {
            while((read = getline(&words[wordCount], &length, file)) != -1) {
                words[wordCount][strlen(words[wordCount])-1] = '\0';
                printf("%s\n", words[wordCount]);
                wordCount+=1;
            }
            fclose(file);
        }

        int i=0;

        n_threads= 0;
        int id = 0;
        int rid;
        pthread_t thread[NUM_THREADS];
        
                //Create socket
        socket_desc = socket(AF_INET , SOCK_STREAM , 0);
        if (socket_desc == -1) {
                printf("Error: Could not create socket\n");
        }
        printf("Socket Created\n");

        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(argv[1]);
        server.sin_port = htons(atoi(argv[2]));

        //Bind
        if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
        //print the error message
                printf("Error: Bind failed.\n");
                return 1;
        }
        printf("Bind Done\n");

        int count=1;
        while(count>0) {
                //clearing buffer
                bzero(t,5);
                //Listen
                listen(socket_desc , 1);

                //Accept and incoming connection
                if(n_threads <= 0){
                        printf("Waiting for connections...\n");
                }

                c = sizeof(struct sockaddr_in);


                //accept connection from an incoming client
                if(n_threads <3){
                        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);

                        if (client_sock < 0) {
                                printf("Error: Accept Failed\n");
                                return 0;
                        }
                        printf("Connection Accepted\n");
                        //clearing the buffer
                        bzero(client_message, 2000);
                        strcpy(client_message,"Ready to start game? (y/n):");

                        //Send the message back to client
                        write(client_sock , client_message, strlen(client_message));
                        recv(client_sock, t, 5, 0);
                        printf("%s\n", t);
                        if(!strcmp(t, "n")) {
                            continue;
                        }
                        n_threads++;
                        rid = pthread_create(&thread[id],NULL,&printThreadID,(void*)(size_t)client_sock);
                        id++;
                    
                        if(rid!=0) {
                                printf("Error: pthread_create() failed for Thread # %d",id);
                                return 0;
                        }

                        if(n_threads <= 0){
                                pthread_exit(NULL);
                                return 0;
                        }
                        id--;
                        printf("%d\n");
                }else {
                        client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
                        if (client_sock < 0) {
                                printf("Error: Accept Failed\n");
                                return 0;
                        }

                        printf("Error: Server overloaded\n");

                        strcpy(client_message,"Error: Server overloaded\n");

                                //Send the message back to client
                        write(client_sock , client_message, strlen(client_message));

                }

        }

        return 0;
}

void *printThreadID(void *id) {
        srand(time(NULL));
        long client_sock = (long)id;
    
        int read_size =0;
        char client_msg[2000],  word[2000];
        char trivia[2000] ;
        memset(trivia, '\0', sizeof(trivia));
        memset(client_msg, '\0', sizeof(client_msg));

        int word_index = randomNum();
        while(words[word_index]==NULL){
                word_index=randomNum();
        }
        strcpy(word, words[word_index]);

        for(int i=0; i<strlen(word); i++) {
                trivia[i] = '_';
        }

        write(client_sock , trivia, strlen(trivia));

        int end = 0;
        
        printf("Game Start Client %i\n",n_threads);
        
        while(end == 0){
                if( (read_size = recv(client_sock , client_msg, 2000 , 0)) > 0 ) {
                        guessChecker(client_msg[0], word, trivia);

                        if(strcmp(client_msg,"end")==0) {
                                write(client_sock , word, strlen(word));
                                n_threads--;
                                pthread_exit(NULL);
                        }

                        if(winGame(trivia))
                                end=1;
                                //Send the message back to client
                        write(client_sock , trivia, strlen(trivia));
                } else {
                        perror("Error: Reveived Failed\n");
                        exit(1);
                }
        }
        write(client_sock, word, strlen(word));
        n_threads--;
        pthread_exit(NULL);
}

int randomNum() {
        int r = rand() % 15;
        return r;
}


void guessChecker(char guess, char w[], char trivia[]) {
        int check=0;

        for(int i=0; i<strlen(w); i++) {

                if(w[i] == guess) {
                        trivia[i] = w[i];
                }
        }
}


int winGame(char trivia[]) {
        int check=1;

        for(int i=0; i<strlen(trivia); i++) {
                if(trivia[i] == '_')
                        check=0;
        }
        return check;
}
