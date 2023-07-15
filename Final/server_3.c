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

/*
mtype
1 - client_id
2 - message
3 - response
4 - termination
*/
typedef struct
{
    long mtype;
    char mtext[MSG_SIZE];
    int client_id;
    int receiver_id;
} Message;

typedef struct
{
    pid_t client_pid;
    int client_mq_id;
    int worker_thread_id;
    int client_id;
} Client;

Client clients[MAX_CLIENTS];
int num_clients = 0;
int server_mq_id;

int mutex=0;

void create_worker_thread(pid_t client_real_mq_id, int client_server_mq_id)
{
    printf("Creating worker thread for client %d\n", client_server_mq_id);
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }
    else if (pid == 0)
    {
        // Worker thread code
        while (1)
        {
            Message msg;
            if (msgrcv(client_server_mq_id, &msg, sizeof(Message), 0, 0) == -1)
            {
                perror("msgrcv failed");
                exit(1);
            }
            printf("==Received message from client %d: %s\n", msg.client_id, msg.mtext);
            // Processing the received message here

            if (msg.mtype == 2)
            {
                while (mutex==1){sleep(0.1);};
                mutex=1;
                
                int receiver_id = msg.receiver_id;

                for (int i = 0; i < num_clients; i++)
                {
                    if (clients[i].client_id == receiver_id)
                    {
                        msg.mtype = 2;
                        if (msgsnd(clients[i].client_mq_id, &msg, sizeof(Message), 0) == -1)
                        {
                            perror("msgsnd failed");
                            exit(1);
                        }
                        break;
                    }
                }

                mutex=0;

            }
            else if (msg.mtype == 4)
            {
                printf("Client %d is shutting down...\n", msg.client_id);
                exit(0);
            }
        }
    }
}

void handle_sigint(int sig)
{
    printf("Server is shutting down...\n");
    // kill all clients
    for (int i = 0; i < num_clients; i++)
    {
        kill(clients[i].client_pid, SIGINT);
        msgctl(clients[i].client_mq_id, IPC_RMID, NULL);
    }
    msgctl(server_mq_id, IPC_RMID, NULL);
    exit(0);
}

int main()
{
    signal(SIGINT, handle_sigint);

    // Create server mailbox
    key_t server_key = ftok("/tmp", 0);
    server_mq_id = msgget(server_key, IPC_CREAT | 0666);

    if (server_mq_id == -1)
    {
        perror("msgget failed");
        exit(1);
    }

    printf("Server is running...\n");

    while (1)
    {

        printf("Waiting for message from client...\n");
        Message msg;

        if (msgrcv(server_mq_id, &msg, sizeof(Message), 1, 0) == -1)
        {

            perror("msgrcv failed");
            exit(1);
        }

        printf("Received message from client\n");
        printf("Message type: %ld\n", msg.mtype);
        if (msg.mtype == 1)
        {
            // Client registration
            pid_t client_mq_id = atoi(msg.mtext);

            printf("Client mailbox id: %d\n", client_mq_id);

            // Create client mailbox for server to recieve
            key_t client_key = ftok("/tmp", msg.client_id);
            int client_server_mq_id = msgget(client_key, IPC_CREAT | 0666);
            if (client_server_mq_id == -1)
            {
                perror("msgget failed");
                exit(1);
            }
            //while (mutex==1){sleep(0.1);};
           // mutex=1;
            // Register the client
            clients[num_clients].client_pid = atoi(msg.mtext);
            clients[num_clients].client_mq_id = client_mq_id;
            clients[num_clients].client_id = msg.client_id;
            create_worker_thread(atoi(msg.mtext), client_server_mq_id);

            printf("Number of clients %d\n", num_clients);
            num_clients++;

            // send mailbox adress to client

            msg.receiver_id = msg.client_id;
            msg.client_id = 0;
            msg.mtype = 3;

            sprintf(msg.mtext, "%d", client_server_mq_id);
            if (msgsnd(client_mq_id, &msg, sizeof(Message), 0) == -1)
            {
                perror("msgsnd failed");
                exit(1);
            }
            printf("Sent response to client\n");
            //mutex=0;
        }
        else if (msg.mtype == 2)
        {
            printf("===Message\n");
            // Client message
            int client_id = msg.client_id;
            char *message = msg.mtext;
            printf("%d: %s", client_id, message);
            msg.client_id = 0;
            msg.mtype = 3;
            strcpy(msg.mtext, "ok");
            if (msgsnd(server_mq_id, &msg, sizeof(Message), 0) == -1)
            {
                perror("msgsnd failed");
                exit(1);
            }
        }
        else if (msg.mtype == 3)
        {
            // Client response
            int client_id = msg.client_id;
            char *message = msg.mtext;
            printf("%d: %s", client_id, message);
        }
        else if (msg.mtype == 4)
        {
            // Client termination

            int client_id = msg.client_id;
            printf("Client %d terminated\n", client_id);

            for (int i = 0; i < num_clients; i++)
            {
                if (clients[i].client_pid == client_id)
                {
                    for (int j = i; j < num_clients - 1; j++)
                    {
                        clients[j] = clients[j + 1];
                    }
                    num_clients--;
                    break;
                }
            }
        }
    }

    return 0;
}