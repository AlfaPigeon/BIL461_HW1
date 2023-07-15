#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define MAX_CLIENTS 5

void handle_client(int new_sock) {
    char client_message[256];
    memset(client_message, 0, sizeof(client_message));
    read(new_sock, client_message, sizeof(client_message));
    printf("Client message: %s\n", client_message);
    close(new_sock);
}

int main(int argc, const char* argv[]) {
    int sock_desc, new_sock, c, *new_sock_ptr;
    struct sockaddr_in server, client;

    //Create socket
    sock_desc = socket(AF_INET, SOCK_STREAM, 0);
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    //Bind
    bind(sock_desc, (struct sockaddr*) &server, sizeof(server));
    //Listen
    listen(sock_desc, MAX_CLIENTS);

    printf("Waiting for incoming connections...\n");
    c = sizeof(struct sockaddr_in);

    while ((new_sock = accept(sock_desc, (struct sockaddr*) &client, (socklen_t*)&c))) {
        printf("Connection accepted\n");
        pid_t pid = fork();
        if (pid == 0) {
            // inside the child process
            handle_client(new_sock);
            return 0;  // end child process
        } else {
            // inside the server (parent) process
            close(new_sock);
        }
    }
    return 0;
}