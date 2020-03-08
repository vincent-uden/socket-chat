#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <netdb.h>

#include "../headers/user.h"
#include "../headers/chat.h"

#define PORT 8080
#define MAX_CLIENTS 30
#define BSIZE 1025

int main() {
    int opt = 1;
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], 
        max_clients = MAX_CLIENTS, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    struct user users[MAX_CLIENTS];

    char buffer[BSIZE];
    
    // Set of socket descriptors
    fd_set readfds;

    char *message = "Echo Daemon v1.0\n";

    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 0;
    struct user u;
    // init all client_socket to 0
    memset(client_socket, 0, MAX_CLIENTS * sizeof (int));
    memset(users, 0, MAX_CLIENTS * sizeof(u));

    // Create master socket
    if ( (master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(1);
    }

    // Set master socket to allow mypltiple connections (not needed but good)
    if ( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                sizeof(opt)) < 0 ) {
        perror("Couldn't set socket options");
        exit(1);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if ( bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0 ) {
        perror("Bind failed");
        exit(1);
    }
    printf("Listening on port %d\n", PORT);

    if ( listen(master_socket, 3) < 0 ) {
        perror("Listen failed");
        exit(1);
    }

    addrlen = sizeof(address);
    printf("Waiting for connections ...\n");

    while ( 1 ) {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        // Add child sockets to set
        for ( i = 0; i < max_clients; i++ ) {
            sd = client_socket[i];

            if ( sd > 0 )
                FD_SET(sd, &readfds);

            if ( sd > max_sd )
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);

        if ( (activity < 0) && (errno!=EINTR) ) {
            printf("Error selecting socket\n");
        }

        // First check if any new connections need to be added
        if ( FD_ISSET(master_socket, &readfds) ) {
            if ( (new_socket = accept(master_socket, 
                    (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0 ) {
                perror("Couldn't accept the new connection");
                exit(1);
            }

            printf("New connection:\n socket id: %d\n ip: %s\n port: %d\n",
                new_socket, inet_ntoa(address.sin_addr), 
                ntohs(address.sin_port)
            );

            if ( send(new_socket, "connected", strlen(message), 0) != strlen(message) ) {
                perror("Error sending message");
            }

            printf("Welcome message sent successfully\n");


            for ( i = 0; i < max_clients; i++ ) {
                if ( client_socket[i] == 0 ) {
                    client_socket[i] = new_socket;
                    break;
                }
            }
        }

        // Handle all client sockets
        for ( i = 0; i < max_clients; i++ ) {
            sd = client_socket[i];

            if ( FD_ISSET(sd, &readfds) ) {
                memset(buffer, 0, BSIZE * sizeof(char));
                if ( (valread = read(sd, buffer, 1024)) == 0 ) {
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    printf("Host disconnected:\n ip: %s\n port: %d\n", 
                        inet_ntoa(address.sin_addr), ntohs(address.sin_port)
                    );

                    close(sd);
                    client_socket[i] = 0;
                    // Todo, remove user
                } else {
                    buffer[valread] = '\0';
                    switch ( buffer[0] ) {
                    case 'c':
                        add_user(buffer, users, sd, i);
                        if ( users[i].sd != sd ) {
                            send(sd, "cA", sizeof("cA"), 0);
                        }
                        break;
                    case 'm':
                        printf("Message from %s! sd: %d %d\n", users[i].username, users[i].sd, sd);
                        send_messages(users[i], users, max_clients, buffer);
                        break;
                    default:
                        printf("%s\n", buffer);

                    };
                }
            }
        }
    }
}
