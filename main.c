/* source code of own Aes implementation in C */
#include "decrypt.h"
#include "network.h"
int key[16],tempArr[16],tempKey[16];
char generatedKey[17],userKey[20]; 
char ciphertext[129] = "";
int menuInput = 0,keyIsSet = 0;

int printY = 1;
int serverFd = -1,clientFd = -1,connectionFd = -1;
char sendBuffer[200],recvBuffer[200];
char plaintext[65];
//functions
int readInt();
void menu();
void sanitize(char* string); 
void keyGen(char generatedKey[17]);
int* convertToIntArr(char* string,int* tempArr);
void *receiveAndDecrypt(void *client);
void *encryptAndSend(void *connection);
void drawInputBorder();

int main(){
    if (sodium_init() < 0){
        puts("Sodium library failed to initialise");
        exit(EXIT_FAILURE);
    }

    pthread_t requestThread,respondThread;

    puts("Enter numbers 1-5 to select from the menu");
    puts("Set the key to use for encryption and decryption before anything else");
    puts("If you can't think of a secure key,you can use the generate key option to generate one for you");
    while (1){
        menu();
        menuInput = readInt();
        if (menuInput < 1 || menuInput > 5){
            puts("Invalid Choice");
            continue;
        }

        if (menuInput == 1){ 
            if (!keyIsSet){
                puts("Please set the key first before starting the server");
                continue;
            }

            int serverPort = 1;
            //generate random port and checking if it can be used
            for (int i = 0;i < 11;i++){
                serverPort = randombytes_uniform(20000) + 10000;
                if (checkValidPort(serverPort) != -1)
                    break;
                if (i == 10){
                    puts("Failed generating random port");
                    exit(EXIT_FAILURE);
                }
            }

            printf("Port Used: %d\n",serverPort);
            serverFd = startServer(serverPort);
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
            
            initscr();
            printw("Connected!\n");
            drawInputBorder();
            refresh();

            pthread_create(&requestThread,NULL,encryptAndSend,(void *)&clientFd);
            pthread_create(&respondThread,NULL,receiveAndDecrypt,(void *)&clientFd);
            pthread_exit(NULL);

            endwin();
            close(clientFd);
            close(serverFd);

            puts("Server Closed");

        } else if(menuInput == 2) {
            if (!keyIsSet){
                puts("Please set the key first before connecting to a server");
                continue;
            }

            char portToConnect[7];
            printf("Port number to use: ");
            fgets(portToConnect,6,stdin);
            sanitize(portToConnect);
            //TODO need to do an ip and port input check here
            connectionFd = connectServer("127.0.0.1",atoi(portToConnect)); 
            
            if (connectionFd == -1){
                puts("Failed to connect to server");
                continue;
            }

            initscr();
            printw("Connected!\n");
            drawInputBorder();
            refresh();

            
            pthread_create(&requestThread,NULL,encryptAndSend,(void *)&connectionFd);
            pthread_create(&respondThread,NULL,receiveAndDecrypt,(void *)&connectionFd);
            pthread_exit(NULL);
            
            
        } else if (menuInput == 3) {
            puts("Please enter the key you want to use to encrypt and decrypt");
            fgets(userKey,17,stdin); 
            sanitize(userKey);
            if ((int)strlen(userKey) != 16){
                puts("Enter a key that is 16 characters long");
                exit(EXIT_FAILURE);
            }
            cpyArray(convertToIntArr(userKey,tempArr),key);
            keyIsSet = true;

        } else if (menuInput == 4){
            keyGen(generatedKey);
            printf("Generated Key: %s\n",generatedKey);

        } else if (menuInput == 5){
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

void *receiveAndDecrypt(void *client){
    int *fdPointer = (int *)client;
    int fd = *fdPointer;
    while (true){
        memset(recvBuffer,'\x00',sizeof(recvBuffer));
        
        read(fd,recvBuffer,128);
        if (recvBuffer[0] == 0)
            break;    

        if (strlen(recvBuffer) > 128){
            puts("Something's wrong");
            exit(EXIT_FAILURE);
        }
        
        cpyArray(key,tempKey); //this line is necessary because the decryption will alter the key and the key can't be used again
        
        attron(A_BOLD);
        mvprintw(printY,0,"%s","Them: ");
        attroff(A_BOLD);
        mvprintw(printY,6,"%s",decrypt(recvBuffer,tempKey)); 

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
        
        attron(A_BOLD);
        mvprintw(printY,0,"%s","You: ");
        attroff(A_BOLD); 
        mvprintw(printY,5,"%s",sendBuffer);
        //clear line
        move(LINES-3,0);
        clrtoeol();
        refresh();
            
        if (strlen(sendBuffer) > 64){
            printw("Something's wrong\n");
            refresh();
            exit(EXIT_FAILURE);
        }

        encrypt(sendBuffer,key,ciphertext);
        if (strlen(ciphertext) > 128){
            printw("Something's wrong\n");
            refresh();
            exit(EXIT_FAILURE);
        }
        write(fd,ciphertext,128);
        printY += 1;
    }
}

void drawInputBorder(){
    int inputBorder1 = LINES-4,inputBorder2 = LINES-1;
    for (int i = 0;i < COLS;i++){
        mvprintw(inputBorder1,i,"=");
        mvprintw(inputBorder2,i,"=");
    }
}

void menu(){
    puts("");
    puts("1: Start server");
    puts("2: Connect server");
    puts("3: Insert key");
    puts("4: Generate random key");
    puts("5: Exit");
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
