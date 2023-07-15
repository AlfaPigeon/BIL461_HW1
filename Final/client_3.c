#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define MSG_SIZE 256

typedef struct {
    long mtype;
    char mtext[MSG_SIZE];
} Message;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <client_id>\n", argv[0]);
        exit(1);
    }

    // Get server's mq_id
    key_t server_key = ftok("/tmp", 's');
    int server_mq_id = msgget(server_key, 0);
    if (server_mq_id == -1) {
        perror("msgget failed");
        exit(1);
    }

    // Inform server of client_id
    Message msg;
    msg.mtype = 1;
    sprintf(msg.mtext, "%d", atoi(argv[1]));
    if (msgsnd(server_mq_id, &msg, sizeof(Message) /*- sizeof(long)*/, 0) == -1) {
        perror("msgsnd failed");
        exit(1);
    }
    printf("Sent client_id to server\n");
    
    // Create client mailbox
    key_t client_key = ftok("/tmp", atoi(argv[1]));
    int client_mq_id = msgget(client_key, IPC_CREAT | 0666);
    if (client_mq_id == -1) {
        perror("msgget failed");
        exit(1);
    }

    while (1) {
        printf("Enter a message to send to the server: ");
        fgets(msg.mtext, MSG_SIZE, stdin);
        msg.mtype = 2;
        if (msgsnd(server_mq_id, &msg, sizeof(Message) /*- sizeof(long)*/, 0) == -1) {
            perror("msgsnd failed");
            exit(1);
        }
        
        // Receive a response
        if (msgrcv(client_mq_id, &msg, sizeof(Message) - sizeof(long), getpid(), 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }
        printf("Received response from server: %s\n", msg.mtext);

        // Code for user to exit process here, e.g using a specific input string
    }

    return 0;
}
