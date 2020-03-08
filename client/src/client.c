#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <sys/time.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "../headers/front-end.h"

#define MAX 1025 
#define PORT 8080 
#define SA struct sockaddr 

char username[41];
int VERBOSE = 0;

void err_exit(char* err_msg) {
    printf("### The program exited unexpectedly ###\nReason: %s\n", err_msg);
    exit(1);
}

/* --- Wrapper functions that format requests to the server --- */
void sock_exit(int sockfd) {
    char* msg = "q";
    write(sockfd, msg, 2 *sizeof(char));
}

void sock_send_msg(int sockfd, char* msg) {
    int msg_len = strlen(msg);
    int output_len = msg_len + 2;
    char output[output_len];
    output[0] = 'm'; output[output_len - 1] = '\0';

    strcpy(output + 1, msg);
    write(sockfd, output, output_len * sizeof(char));
}
/* --- End of wrapper functions --- */

/* --- Functions that handle requests from the server --- */
void handle_msg(char* req) {
    int req_len = strlen(req);
    int name_len = 0, msg_len = 0;
    for (int i = 1; req[i] != '|'; i++) {
        if (i == req_len) { err_exit("Malformed request from server"); }
        name_len++;}
    for (int i = name_len + 2; req[i] != '\0'; i++) {
        if (i > req_len) { err_exit("Malformed request from server"); }
        msg_len++;
    }

    char name[name_len + 1]; name[name_len] = '\0';
    char msg[msg_len + 1]; msg[msg_len] = '\0';
    strncpy(name, req + 1, name_len);
    strncpy(msg, req + 1 + name_len + 1, msg_len);

    display_message(name, msg);
}
/* --- End of request handling functions --- */

void chat_main(int sockfd) {
	char sock_buff[MAX]; 
    char stdin_buff[MAX];
    int running = 1;

    // Variables for select()
    fd_set all_set, r_set;
    int maxfd = sockfd + 1;
    FD_ZERO(&all_set);

    FD_SET(STDIN_FILENO, &all_set);
    FD_SET(sockfd, &all_set);

    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    char c; // Used for reading input in loop

    while ( running ) {
        r_set = all_set;
        memset(sock_buff, 0, MAX * sizeof(char));
        memset(stdin_buff, 0, MAX * sizeof(char));
        memset(username, 0, 41 * sizeof(char));

        select(maxfd, &r_set, NULL, NULL, &tv);

        if ( FD_ISSET(STDIN_FILENO, &r_set) ) {
            read(STDIN_FILENO, stdin_buff, sizeof(char) * MAX);

            if (strcmp(stdin_buff, "exit\n") == 0) {
                running = 0;
                sock_exit(sockfd);
            } else {
                sock_send_msg(sockfd, stdin_buff);
            }
        }

        if ( FD_ISSET(sockfd, &r_set) ) {
            read(sockfd, sock_buff, sizeof(sock_buff));
            if (VERBOSE) {
                printf("Reading socket\nFrom Server : %s\n", sock_buff);
            }

            switch ( sock_buff[0] ) {
            case 'c':
                if (VERBOSE)
                    puts("Server accepted connection");

                memset(sock_buff, 0, MAX * sizeof(char)); 
                sock_buff[0] = 'c';

                printf("Please enter your username: ");
                for (int i = 0; (c = getchar()) != '\n' && i < 40; i++) {
                    username[i] = c;
                }

                strcpy(sock_buff + 1, username);
                write(sockfd, sock_buff, sizeof(sock_buff));
                break;
            case 'm':
                if (VERBOSE)
                    puts("Message from server");

                if (VERBOSE > 1)
                    printf("Server request: %s\n", sock_buff);

                handle_msg(sock_buff);
            default:
                if ((strncmp(sock_buff, "q", 1)) == 0) {
                    printf("The server disconnected.\n");
                    running = 0;
                }
                break;
            }
        }
    }
} 

int main() { 
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        err_exit("Socket creation failed");
    }
    else
        printf("Socket successfully created..\n");
    memset(&servaddr, 0, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        err_exit("Connection with the server failed");
    }
    else
        puts("Connected to the server\n");

    chat_main(sockfd);

    close(sockfd);
} 

