#include <sys/socket.h>
#include <arpa/inet.h>
#include "header.h"

//functions
int startServer(int port);
int connectServer(char ip[20],int port);
int main(){
    int serverFd = -1,clientFd = -1;
    char sendBuffer[1025],recvBuffer[1025];
    serverFd = startServer(5000);
    if (serverFd == -1){
        printf("Server failed to start");
        exit(EXIT_FAILURE);
    }
    puts("Waiting for connections");
    clientFd = accept(serverFd,NULL,NULL);

    strcpy(sendBuffer,"Connection made!");
    write(clientFd,sendBuffer,strlen(sendBuffer));
    close(serverFd);
    
    /*clientFd = connectServer("127.0.0.1",5000);
    if (clientFd == -1)
        exit(EXIT_FAILURE);

    read(clientFd,recvBuffer,1024);
    recvBuffer[strlen(recvBuffer) - 1] = 0;
    printf("%s\n",recvBuffer);
    close(clientFd);*/
}

int startServer(int port){
    //create socket and bind socket to port
    int fd = -1;
    fd = socket(AF_INET,SOCK_STREAM,0);
    //this struct is needed for bind to specify which port
    struct sockaddr_in server;
    memset(&server,'0',sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if (fd == -1){
        puts("Failed to create socket");
        return -1;
    }
            
    if (bind(fd,(struct sockaddr*)&server,sizeof(server)) == -1){
        puts("Socket failed to bind");
        return -1;
    }
    
    //listen for a connection and accept
    if (listen(fd,10) == -1){
        puts("Server failed to listen");
        return -1;
    }
    
    return fd;
}

int connectServer(char ip[20],int port){
    int fd = -1;
    struct sockaddr_in client;

    fd = socket(AF_INET,SOCK_STREAM,0);

    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = inet_addr(ip);

    if (connect(fd,(struct sockaddr *)&client,sizeof(client)) == -1){
        puts("Failed to Connect");
        return -1;
    }

    return fd;
}
