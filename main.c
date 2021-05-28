/* source code of own Aes implementation in C */
#include "encrypt.h"

int plaintext[16],key[17],tempArr[16];
char generatedKey[16],userKey[16],convertStr[16],userInput[65];
char ciphertext[150] = "";
//functions
int* convertToIntArr(char* string,int* tempArr);
char* keyGen(char generatedKey[16]);
void sanitize(char* string); //bool

int main(){
    printf("Please enter the text you want to encrypt\n");
    fgets(userInput,65,stdin); //extra one char for null byte ,string ends in \x00 
    sanitize(userInput);

    printf("Please enter the key you want to use to encrypt\n");
    fgets(userKey,17,stdin); 
    sanitize(userKey);
    if ((int)strlen(userKey) != 16){
        printf("Enter a key that is 16 characters long\n");
        exit(EXIT_FAILURE);
    }
    
    int times = ceil((double)strlen(userInput) / 16);
    char tempString[5];
    for (int i = 0;i < times;i++){
        for (int j = 0;j < 16;j++){
            convertStr[j] = userInput[j + 16*i];
        }
        cpyArray(convertToIntArr(convertStr,tempArr),plaintext);
        cpyArray(convertToIntArr(userKey,tempArr),key);
        encrypt(plaintext,key);
        //concatenate all into one string
        for (int index = 0;index < 16;index++){
            sprintf(tempString,"%02x",plaintext[index]);
            strcat(ciphertext,tempString);
        }
        printHex(plaintext);
    }
    printf("%s\n",ciphertext);
    
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
    char printable[95] = " !\"#$%&'()*+,-./0123456789:;<=>?@1ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}";
    srand(time(NULL)); //set the seed for random to current time
    for (int i = 0;i <= 15;i++){
        int index = floor(((double)rand()/RAND_MAX) * 94);
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
