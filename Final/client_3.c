#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#define MSG_SIZE 256

typedef struct
{
    long mtype;
    char mtext[MSG_SIZE];
    int client_id;
    int receiver_id;
} Message;
int client_id = 0;
int receiver_id = 0;
void sigint_handler(int signo)
{
    printf("Exiting client\n");
    // Get server's mq_id
    key_t server_key = ftok("/tmp", 0);
    int server_mq_id = msgget(server_key, 0);
    if (server_mq_id == -1)
    {
        printf("Server Shutdown\n");
        exit(0);
    }

    Message msg;
    msg.mtype = 4;
    msg.client_id = client_id;
    sprintf(msg.mtext, "%d", client_id);
    if (msgsnd(server_mq_id, &msg, sizeof(Message), 0) == -1)
    {
        printf("Server Shutdown\n");
        exit(0);
    }
    exit(0);
}
void create_worker_thread(pid_t client_mq_id)
{
    // This thread for output
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        while (1)
        {

            Message msg;

            if (msgrcv(client_mq_id, &msg, sizeof(Message), 0, 0) == -1)
            {
                perror("msgrcv failed");
                exit(1);
            }
            // Not from server
            if (msg.client_id != 0)
            {
                printf("%d: %s\n", msg.client_id, msg.mtext);
            }
        }
    }
}
int main(int argc, char **argv)
{
    signal(SIGINT, sigint_handler);
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <client_id> <reciever_id>\\n", argv[0]);
        exit(1);
    }

    client_id = atoi(argv[1]);
    // Create client mailbox
    key_t client_key = ftok("/tmp", client_id + 1000);
    int client_mq_id = msgget(client_key, IPC_CREAT | 0666);
    if (client_mq_id == -1)
    {
        perror("msgget failed");
        exit(1);
    }
    printf("Client: %d\n", client_mq_id);

    printf("Client mailbox created\n");

    // Get server's mq_id
    key_t server_key = ftok("/tmp", 0);
    int server_mq_id = msgget(server_key, 0);
    if (server_mq_id == -1)
    {
        perror("msgget failed");
        exit(1);
    }

    // Inform server of client_id
    Message msg;
    msg.client_id = client_id;
    msg.mtype = 1;
    msg.receiver_id = 0;

    sprintf(msg.mtext, "%d", client_mq_id);
    if (msgsnd(server_mq_id, &msg, sizeof(Message), 0) == -1)
    {
        perror("msgsnd failed");
        exit(1);
    }
    printf("Sent client_id to server\n");

    printf("Waiting for server to send client_id\n");

    if (msgrcv(client_mq_id, &msg, sizeof(Message), 0, 0) == -1)
    {

        perror("msgrcv failed");
        exit(1);
    }
    printf("Received client_id from server: %s\n", msg.mtext);
    int server_client_mq_id = atoi(msg.mtext);
    create_worker_thread(client_mq_id);
    while (1)
    {
        msg.client_id = client_id;
        printf("Enter a message to send to the server: \n");
        msg.mtype = 2;

        scanf("%s", msg.mtext);
        /*fflush(stdin);
        printf("Enter receiver id: \n");

        scanf("%d", &msg.receiver_id);*/
        msg.receiver_id = atoi(argv[2]);
        if (msgsnd(server_client_mq_id, &msg, sizeof(Message), 0) == -1)
        {
            perror("msgsnd failed");
            exit(1);
        }

        printf("\n");
        sleep(1);
    }

    return 0;
}
