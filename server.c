#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MaxClients 10

typedef struct {
    int id;
    int mailbox_fd;
} Client;

typedef struct {
    int id;
    char* message;
} Message;

void client_process(void *arg){
    Client *client = (Client *)arg;
    printf("Client %d\n", client->id);
    printf("PID %d\n", getpid());
    exit(0);
}


int main(){

    int mailbox_fd = shm_open("/mailbox", O_CREAT | O_RDWR, 0666);
    ftruncate(mailbox_fd, sizeof(Message));

    Message *mailbox = mmap(NULL, sizeof(Message), PROT_READ | PROT_WRITE, MAP_SHARED, mailbox_fd, 0);



    int client_id = 0;



    while(1){
        if(mailbox->id != client_id){
            Client client;
            client.id = client_id;
            client.mailbox_fd = mailbox_fd;
            client_process(&client);
        }
        client_id++;

        if(client_id == MaxClients){
            client_id = 0;
        }

        sleep(1);
    }

    shm_unlink("/mailbox");
    return 0;
}
