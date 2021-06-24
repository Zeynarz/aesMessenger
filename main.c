/* source code of own Aes implementation in C */
#include "decrypt.h"
int plaintext[16],key[16],tempArr[16],tempKey[16];
char generatedKey[17],userKey[20],convertStr[17],userInput[70],userCipher[129]; 
char ciphertext[129] = "";
int menuInput,keyIsSet = 0;
//functions
int readInt();
void menu();
void sanitize(char* string); 
int* convertToIntArr(char* string,int* tempArr);
char* keyGen(char generatedKey[17]);

int main(){
    puts("Enter numbers 1-5 to select from the menu");
    puts("Set the key to use for encryption and decryption before anything else");
    puts("If you can't think of a good and secure key,you can use the generate key option to generate one for you");
    while (1){
        menu();
        menuInput = readInt();
        if (menuInput < 1 || menuInput > 5){
            puts("Invalid Choice");
            continue;
        }

        if (menuInput == 1){ 
            if (!keyIsSet){
                puts("Please set the key first before encrypting");
                continue;
            }
            puts("Please enter the text you want to encrypt");
            fgets(userInput,65,stdin); 
            sanitize(userInput);
            
            //encrypt user string into one big cipher text
            int times = ceil((double)strlen(userInput) / 16);
            char tempString[5];
            for (int i = 0;i < times;i++){
                for (int j = 0;j < 16;j++){
                    convertStr[j] = userInput[j + 16*i];
                }
                cpyArray(convertToIntArr(convertStr,tempArr),plaintext);
                cpyArray(key,tempKey); //this line is necessary because the encryption/decryption altered the key and the key can't be used again
                encrypt(plaintext,tempKey);
                //concatenate all into one string
                for (int index = 0;index < 16;index++){
                    sprintf(tempString,"%02x",plaintext[index]);
                    strcat(ciphertext,tempString);
                }
            }
            printf("%s\n",ciphertext);

        } else if(menuInput == 2) {
            if (!keyIsSet){
                puts("Please set the key first before decrypting");
                continue;
            }

            puts("Please enter the cipher text you want to decrypt");
            fgets(userCipher,129,stdin);
            sanitize(userCipher);
            if (strlen(userCipher) % 32 != 0){
                puts("Enter a valid cipher text (length divisible by 32)");
                exit(EXIT_FAILURE);
            }
            cpyArray(key,tempKey); 
            printf("%s\n",decrypt(userCipher,tempKey));

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
char* keyGen(char generatedKey[16]){
    int index,randomNum;
    char printable[96] = " !\"#$%&'()*+,-./0123456789:;<=>?@1ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";
    for (int i = 0;i <= 15;i++){
        getrandom(&randomNum,1,GRND_NONBLOCK); //get random num from /dev/urandom
        index = floor(((double)randomNum/255) * 94);
        generatedKey[i] = printable[index];
    }
    return generatedKey;
}
void sanitize(char* string){
    int inputLen = strlen(string);
    //remove \n char if there is one
    if (string[inputLen - 1] == 10){
        string[inputLen - 1] = 0;
        inputLen -= 1;
    } else {
        //read in all the bytes
        while (getchar() != 10)
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

void menu(){
    puts("1: Encrypt");
    puts("2: Decrypt");
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
