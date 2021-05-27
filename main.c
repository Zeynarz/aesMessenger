/* source code of own Aes implementation in C */
#include "encrypt.h"
int plaintext[16],key[16],tempArr[16];
char generatedKey[16];
//functions
int* convertToIntArr(char* string,int* tempArr);
char* keyGen(char generatedKey[16]);

int main(){
    cpyArray(convertToIntArr("ABCDABCDABCDABCD",tempArr),plaintext);
    cpyArray(convertToIntArr("EFGHEFGHEFGHEFGH",tempArr),key);
    encrypt(plaintext,key);
    printf("%s\n",keyGen(generatedKey));
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
