#include "aesDecrypt.h"
#include "network.h"
int key[16],tempArr[16],tempKey[16];
int menuInput = 0,keyIsSet = false,usernameIsSet = false;

char generatedKey[17],userKey[20]; 
char ciphertext[129] = "",plaintext[65] = "";

char username[20] = "",connectionUsername[20] = "";
char tempUsername [21] = ""; //used for checking received username

//variables used in networking
int printY = 1;
int serverFd = -1,clientFd = -1,connectionFd = -1;
char sendBuffer[200],recvBuffer[200];
char leaveString[] = "LEAVE";

pthread_t requestThread,respondThread;
//key functions
int readInt();
void menu();
void sanitize(char* string); 
void keyGen(char generatedKey[17]);
int* convertToIntArr(char* string,int* tempArr);
void *receiveAndDecrypt(void *connection);
void *encryptAndSend(void *connection);

//functions to reduce repeated code 
void drawInputBorder();
void printYReset();
void sendAndReceiveUsername(int sendReceiveFd);
void initialise(int fd);

int main(){
    if (sodium_init() < 0){
        puts("Sodium library failed to initialise");
        exit(EXIT_FAILURE);
    }


    puts("Enter numbers 1-6 to select from the menu");
    puts("Set the key to use for encryption and decryption before anything else");
    puts("If you can't think of a secure key,you can use the generate key option to generate one for you");
    while (1){
        menu();
        menuInput = readInt();
        if (menuInput < 1 || menuInput > 6){
            puts("Invalid Choice");
            continue;
        }

        if (menuInput == 1){ 
            if (!keyIsSet){
                puts("Please set the key first before starting the server");
                continue;
            } else if (!usernameIsSet) {
                puts("Please set your username before starting the server");
                continue;
            }

            char serverPort[7];
            printf("Port to use: ");
            fgets(serverPort,6,stdin);
            sanitize(serverPort);
            if (checkValidPort(atoi(serverPort)) == -1){
                puts("The port is busy");
                memset(serverPort,'\x00',sizeof(serverPort));
                break;
            }

            serverFd = startServer(atoi(serverPort));
            if (serverFd == -1){
                puts("Server failed to start");
                exit(EXIT_FAILURE);
            }
            
            puts("Waiting for a connection");
            clientFd = accept(serverFd,NULL,NULL);
            if (clientFd == -1){
                puts("Client failed to Connect");
                continue;
            }
          
            initialise(clientFd);

            pthread_create(&requestThread,NULL,encryptAndSend,(void *)&clientFd);
            pthread_create(&respondThread,NULL,receiveAndDecrypt,(void *)&clientFd);
           
            //wait till request and respond thread terminate
            pthread_join(requestThread, NULL);
            pthread_join(respondThread, NULL);

            printY = 1;
            endwin();
            close(clientFd);
            close(serverFd);

            puts("Server Closed");

        } else if(menuInput == 2) {
            if (!keyIsSet){
                puts("Please set the key first before connecting to a server");
                continue;
            } else if (!usernameIsSet) {
                puts("Please set your username before connecting to a server");
                continue;
            }

            char portToConnect[7],ip[17];
            printf("Ip: ");
            fgets(ip,16,stdin);
            sanitize(ip);

            printf("Port number to use: ");
            fgets(portToConnect,6,stdin);
            sanitize(portToConnect);

            if (strlen(portToConnect) > 5 || strlen(ip) > 15){
                puts("Invalid ip/port");
                memset(portToConnect,'\x00',sizeof(portToConnect));
                memset(ip,'\x00',sizeof(ip));
                continue;
            }

            puts("Trying to connect");
            connectionFd = connectServer("127.0.0.1",atoi(portToConnect)); 
            
            if (connectionFd == -1){
                puts("Connection failed");
                memset(portToConnect,'\x00',sizeof(portToConnect));
                memset(ip,'\x00',sizeof(ip));
                continue;
            }
           
            initialise(connectionFd);
            
            pthread_create(&requestThread,NULL,encryptAndSend,(void *)&connectionFd);
            pthread_create(&respondThread,NULL,receiveAndDecrypt,(void *)&connectionFd);
            
            pthread_join(requestThread, NULL);
            pthread_join(respondThread, NULL);

            printY = 1;
            endwin();
            close(connectionFd);

        } else if (menuInput == 3) {
            puts("Please enter the key you want to use to encrypt and decrypt");
            fgets(userKey,17,stdin); 
            sanitize(userKey);
            if ((int)strlen(userKey) != 16){
                puts("Enter a key that is 16 characters long");
                exit(EXIT_FAILURE);
            }
            
            printf("Using Key: %s\n",userKey);
            cpyArray(convertToIntArr(userKey,tempArr),key);
            keyIsSet = true;

        } else if (menuInput == 4){ 
            puts("Please enter your username (maximum 16 characters)");
            fgets(username,17,stdin);
            sanitize(username);

            if (strlen(username) > 16){
                puts("Maximum username length is 16 characters");
                memset(username,'\x00',sizeof(username));
                continue;
            }

            printf("Username: %s\n",username);
            usernameIsSet = true;

        } else if (menuInput == 5){
            keyGen(generatedKey);
            printf("Generated Key: %s\n",generatedKey);

        } else if (menuInput == 6){
            puts("Bye!");
            exit(EXIT_SUCCESS);
        }
    }
}

int* convertToIntArr(char* string,int* tempArr){
    int len = strlen(string);
    for (int i = 0;i <= 15;i++){
        if (len > i){
            tempArr[i] = string[i];
        } else {
            tempArr[i] = 0;
        }
    }
    return tempArr;
}

void keyGen(char generatedKey[17]){
    int index;
    char printable[95] = "!\"#$%&'()*+,-./0123456789:;<=>?@1ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";

    for (int i = 0;i <= 15;i++){ 
        index = randombytes_uniform(94); //use sodium.h to generate more secure random numbers
        generatedKey[i] = printable[index];
    }
}

void sanitize(char* string){
    int inputLen = strlen(string);
    char placeholder[3] = "";
    //remove \n char if there is one
    if (string[inputLen - 1] == 10){
        string[inputLen - 1] = 0;
        inputLen -= 1;
    } else {
        //read in all the bytes 
        while(getchar() != 10)
            continue;
        
    }
    //sanitize input so only printable chars are left
    for (int i = 0;i < inputLen;i++){
        if (string[i] > 126 || string[i] < 32){
            printf("Enter only Ascii printable characters\n");
            exit(EXIT_FAILURE);
        }
    }
}

void *receiveAndDecrypt(void *connection){
    int *fdPointer = (int *)connection;
    int fd = *fdPointer;

    while (true){
        memset(recvBuffer,'\x00',sizeof(recvBuffer));

        read(fd,recvBuffer,128);
        //check if they typed /leave or if the connection is lost
        if (strncmp(recvBuffer,leaveString,strlen(leaveString)) == 0 || recvBuffer[0] == 0){
            attron(A_BOLD);
            if (printY > LINES-5)
                printYReset();
            
            mvprintw(printY,0,"%s","Him/Her has disconnected  / Both of you dont have matching keys");
            mvprintw(printY+1,0,"%s","Press any key to leave");
            attroff(A_BOLD);
            
            refresh();
            pthread_cancel(requestThread);
            
            move(LINES-3,0);
            getch();
            break;
        }

        if (strlen(recvBuffer) > 128){
            endwin();
            puts("Something's wrong");
            exit(EXIT_FAILURE);
        }
        
        //make sure where im printing doesnt write over the input border 
        if (printY > LINES-5)
            printYReset();

        //this line is necessary because the decryption will alter the key and the key can't be used again
        cpyArray(key,tempKey); 
        
        attron(A_BOLD);
        mvprintw(printY,0,"%s: ",connectionUsername);
        attroff(A_BOLD);
        mvprintw(printY,strlen(connectionUsername) + 2,"%s",aesDecrypt(recvBuffer,tempKey)); 

        move(LINES-3,0);
        refresh();

        printY += 1;
    }
}

void *encryptAndSend(void *connection){
    int *fdPointer = (int *)connection;
    int fd = *fdPointer;
    while (true){
        memset(sendBuffer,'\x00',sizeof(sendBuffer));
        memset(ciphertext,'\x00',sizeof(ciphertext));

        //the function receive 64 chars as input,doesnt include \n byte in string,so sendBuffer[len - 1] = '\x00'
        mvgetnstr(LINES-3,0,sendBuffer,64);

        //dont allow to send empty strings
        if (sendBuffer[0] == 0)
            continue;

        if (printY > LINES-5)
            printYReset();

        attron(A_BOLD);
        mvprintw(printY,0,"%s: ",username);
        attroff(A_BOLD); 
        mvprintw(printY,strlen(username) + 2,"%s",sendBuffer);

        //clear line
        move(LINES-3,0);
        clrtoeol();
        refresh();
        
        if (strlen(sendBuffer) > 64){
            endwin();
            puts("Something's wrong");
            exit(EXIT_FAILURE);
        }

        if (strncmp(sendBuffer,"/leave",6) == 0){
            write(fd,leaveString,strlen(leaveString));
            pthread_cancel(respondThread);
            break;
        }

        aesEncrypt(sendBuffer,key,ciphertext);
        if (strlen(ciphertext) > 128){
            printw("Something's wrong\n");
            refresh();
            exit(EXIT_FAILURE);
        }
        write(fd,ciphertext,128);
        printY += 1;
    }
}

void sendAndReceiveUsername(int sendReceiveFd){
    memset(tempUsername,'\x00',sizeof(tempUsername));

    strncpy(tempUsername,username,strlen(username));
    for (int i = 1;i < 17;i++)
        username[i] = tempUsername[i-1]; 
    username[0] = '#';

    //send #username to connection and revert username back to normal
    write(sendReceiveFd,username,17);
    strncpy(username,tempUsername,16);

    memset(tempUsername,'\x00',sizeof(tempUsername));
    read(sendReceiveFd,tempUsername,17);

    //let the write reach the read() in sendAndReceiveUsernmae,if I dont do this the username I sent will be received by the read() in receiveAndDecrypt and and error will occur
    sleep(1);

    if (tempUsername[0] != '#' || strlen(tempUsername) > 17){
        mvprintw(printY,0,"%s","Failed to get the username of the person connected so they will be reffered to \"Them\""); 
        printY++;
        strncpy(connectionUsername,"Them",4);
    } else {
        strncpy(connectionUsername,tempUsername+1,16);
    }
}

void initialise(int fd){
    initscr();

    attron(A_BOLD);
    sendAndReceiveUsername(fd); 
    printw("Connected to %s!",connectionUsername);
    attroff(A_BOLD);

    drawInputBorder();
    refresh();
}

void drawInputBorder(){
    int inputBorder1 = LINES-4,inputBorder2 = LINES-1;
    for (int i = 0;i < COLS;i++){
        mvprintw(inputBorder1,i,"=");
        mvprintw(inputBorder2,i,"=");
    }
}

void printYReset(){
    clear();
    drawInputBorder();
    printY = 1;
    move(LINES-3,0);
}

void menu(){
    puts("");
    puts("1: Start server");
    puts("2: Connect server");
    puts("3: Insert key");
    puts("4: Insert username");
    puts("5: Generate random key");
    puts("6: Exit");
    printf("Select: ");
}

int readInt(){
    int input = getchar() - 48;
    if (input != (10 - 48)){
        while (getchar() != 10)
            continue;
    }
    return input;
}
