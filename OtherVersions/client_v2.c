#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define CLIENT_MAILBOX "client_mailbox"
#define SERVER_MAILBOX "server_mailbox"
#define SIZE_MAX 256

int main(int argc, char *argv[]) {   // command ekranında çalıştırılırken  $./a.out (istemciName) yazılır ve argv[1] istemcinin mesajlaşmak                                       için kullanacağı kimlik olur
    
    char[20] clientName = argv[1];
    int operation, server_box, client_box;;
    char[20] targetName;
    char[SIZE_MAX] message;

     
    if( mkfifo(CLIENT_MAILBOX, 0666) == -1){                            // creating mailbox
        perror("Client mailbox could not be created");
        exit(1);
    }      
    
    server_box = open(SERVER_MAILBOX, O_WRONLY);                 // open write only server mailbox
    
    if (server_box == -1) {        
        perror("Server mailbox could not be opened");
        exit(1);
    }

    client_box = open(CLIENT_MAILBOX, O_RDONLY);                     // open read only client mailbox
    
    if (client_box == -1) {           
        perror("Client mailbox could not be opened");
        exit(1);
    }
     
    while(1){
        
        printf("\nSelect your operation : 1= Send message, 2= Exit ");
        scanf("%d", &operation);
        
        
        if (operation == 2){
            printf("\nClient Exit");
            exit(0);
        }
        
        else if(operation == 1){
            
        printf("Enter a message to send to the server: ");
        fgets(message, SIZE_MAX, stdin);
        write(serverMailbox, message, strlen(message) + 1);
                
        printf("\nTarget client name : ");
        scanf("%[^\n]s", targetName);
        
        printf("\nType your message : ");
        scanf("%[^\n]s", message);
                
        read(client_box, message, SIZE_MAX);
        printf("Server response: %s\n", message);

        close(client_box);
        close(server_box);
        unlink(CLIENT_MAILBOX);
        
        }
        
        else {
            printf("\nType 1 or 2 to select operation");
            continue;
        }

    }

    return 0;
}