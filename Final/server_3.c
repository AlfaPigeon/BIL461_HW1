#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>


#define MAX_CLIENTS 10
#define MSG_SIZE 256


typedef struct {
    long mtype;
    char mtext[MSG_SIZE];
} Message;


typedef struct {
    pid_t client_pid;
    int client_mq_id;
    int worker_thread_id;
} Client;


Client clients[MAX_CLIENTS];
int num_clients = 0;
int server_mq_id;


void create_worker_thread(pid_t client_pid, int client_mq_id) {
    printf("Creating worker thread for client %d\n", client_mq_id);
    pid_t pid = fork();
    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Worker thread code
        while (1) {
            Message msg;
            if (msgrcv(client_mq_id, &msg, sizeof(Message) - sizeof(long), getpid(), 0) == -1) {
                printf("client reached here\n");
                perror("msgrcv failed");
                exit(1);
            }
            printf("Received message from client %d: %s\n", client_pid, msg.mtext);
            // Process the received message here
            
            // Send a response back to the client
            strcpy(msg.mtext, "This is a response from the server");
            msg.mtype = client_pid;
            if (msgsnd(client_mq_id, &msg, sizeof(Message) - sizeof(long), 0) == -1) {
                perror("msgsnd failed");
                exit(1);
            }
        }
    }
}


void handle_sigint(int sig) {
    printf("Server is shutting down...\n");
    for (int i = 0; i < num_clients; i++) {
        kill(clients[i].client_pid, SIGINT);
        msgctl(clients[i].client_mq_id, IPC_RMID, NULL);
    }
    msgctl(server_mq_id, IPC_RMID, NULL);
    exit(0);
}


int main() {
    signal(SIGINT, handle_sigint);


    // Create server mailbox
    key_t server_key = ftok("/tmp", 's');
    server_mq_id = msgget(server_key, IPC_CREAT | 0666);

    if (server_mq_id == -1) {
        perror("msgget failed");
        exit(1);
    }


    printf("Server is running...\n");


    while (1) {
        printf("Waiting for a new client...\n");
        Message msg;
        if (msgrcv(server_mq_id, &msg, sizeof(Message) /*- sizeof(int)*/, 1, 0) == -1) {
            perror("msgrcv failed");
            exit(1);
        }
        printf("Received message from client: %s\n", msg.mtext);
        printf("Client PID: %d\n", (int)msg.mtype);
        pid_t client_pid = atoi(msg.mtext);


        // Create client mailbox
        key_t client_key = ftok("/tmp", client_pid);
        int client_mq_id = msgget(client_key, IPC_CREAT | 0666);
        if (client_mq_id == -1) {
            perror("msgget failed");
            exit(1);
        }


        // Register the client
        clients[num_clients].client_pid = client_pid;
        clients[num_clients].client_mq_id = client_mq_id;
        create_worker_thread(client_pid, client_mq_id);

        printf("Number of clients %d\n", num_clients);
        num_clients++;
    }


    return 0;
}