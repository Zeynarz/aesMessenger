#include "header.h"
//operation functions
int* subBytes(int word[16],int isRotWord);
int* keySchedule(int key[16],int round); 
int* addRoundKey(int plaintext[16],int key[16]);
int* shiftRows(int plaintext[16]); 
int* mixColumns(int plaintext[16]);
int* cpyArray(int src[16],int dest[16]);
int* encrypt(int plaintext[16],int key[16]);
int convertToByte(int target);

//testing functions
void printHex(int word[16]); 

int* encrypt(int plaintext[16],int key[16]){
    //before round start
    addRoundKey(plaintext,key);
    //main rounds
    for (int i = 1; i <= 9;i++){
        subBytes(plaintext,false);
        shiftRows(plaintext);
        mixColumns(plaintext);
        keySchedule(key,i);
        addRoundKey(plaintext,key);
    }
    //after main rounds
    keySchedule(key,10);
    subBytes(plaintext,false);
    shiftRows(plaintext);
    addRoundKey(plaintext,key);
    return plaintext;
}

int* subBytes(int word[16],int isRotWord){ //isRotWord is treated like a bool
    int subBox[16][16] = {
    {0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76},
    {0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0},
    {0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15},
    {0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75},
    {0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84},
    {0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf},
    {0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8},
    {0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2},
    {0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73},
    {0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb},
    {0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79},
    {0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08},
    {0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a},
    {0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e},
    {0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf},
    {0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16}
};
    char hex[100] = "";
    int indexes[100];
    char finalStr[100] = "";
    char charInt;
    //Convert int to hex and strore in hex variable
    //one hexstring takes twobytes
    //JKLM will become 4a4b4c4d
    if (isRotWord == false){
        for (int i = 0 , plusHex = 0; i < 16; ++i , plusHex += 2){
            sprintf(hex + plusHex,"%02x",word[i] & 0xff);
        }
    } else {
        for (int i = 0 , plusHex = 0; i < 4; ++i , plusHex += 2){
            sprintf(hex + plusHex,"%02x",word[i] & 0xff);
        }
    }
    //convert hex to decimal for example a will be 16 but 1 will still be 1 so it could be used as indexes in array
    //4a4b4c4d will become {4,10,4,11,4,12,4,13}
    for (int i=0; i < strlen(hex); i++){
         if (!isdigit(hex[i])){ 
            indexes[i] = hex[i] - 97 + 10;
         } else {
            indexes[i] = hex[i] - 48;
         }
    }
    for (int i = 0, j = 0; i < strlen(hex); i += 2,j++) //strlen(hex) is probably 32
       word[j] = subBox[indexes[i]][indexes[i+1]];
    
    return word;
}

int* keySchedule(int roundkey[16],int round){
    /*get the last row of the key*/
    int rotword[4];
    int firstLetter;
    for (int counter = 12;counter <= 15;counter++){
        rotword[counter % 12] = roundkey[counter];
    }
    
    firstLetter = rotword[0];
    for (int i = 0; i < 3; i++){
        rotword[i] = rotword[i + 1];
    }
    rotword[3] = firstLetter;
    subBytes(rotword,true);

    //xor
    int rc = pow(2,round - 1);
    if (round >= 9){
        rc ^= 0x11b;
        if (round == 10){
            rc = 32 + 16 + 4 + 2;
        }
    }
    roundkey[0] = roundkey[0] ^ rotword[0] ^ rc; //^ round constant 
    for (int i = 1;i <= 15;i++){
        if (i < 4){
            roundkey[i] = convertToByte(roundkey[i] ^ rotword[i]); 
        } else {
            roundkey[i] = convertToByte(roundkey[i] ^ roundkey[i - 4]);
        }
    }

    return roundkey;
}

int* shiftRows(int word[16]){
    int temp[16];
    //cpy array
    cpyArray(word,temp);
    for (int index = 1;index < 4;index++){
        word[index] = temp[index+4];
        word[index+4] = temp[index+8];
        word[index+8] = temp[index+12];
        word[index+12] = temp[index];
        cpyArray(word,temp);
    }
    for (int index = 2;index < 4;index++){
        word[index] = temp[index+4];
        word[index+4] = temp[index+8];
        word[index+8] = temp[index+12];
        word[index+12] = temp[index];
        cpyArray(word,temp);
    }
    int index = 3;
    word[index] = temp[index+4];
    word[index+4] = temp[index+8];
    word[index+8] = temp[index+12];
    word[index+12] = temp[index];
    return word;
}

int* mixColumns(int plaintext[16]){
    int matrix[16] = {
              2,3,1,1,
              1,2,3,1,
              1,1,2,3,
              3,1,1,2
        };
    int temp[16];
    int current;
    int letter,num,product;
    int final = 0;
    for (int j = 0;j < 4;j++){ //Whole thing
        for (int n = 0;n < 4;n++){ //A column
            for (int i = 0;i < 4;i++){ //One letter
                letter = plaintext[i+j*4];  
                num = matrix[i+n*4];
                product = 0;
                //Galois Field multiplication
                if (num == 1){
                    product = letter;
                } else if (num == 2 || num == 3){
                    if (letter < 0x80){
                        product = letter<<1;
                    } else {
                        product = (letter << 1) ^ 0x1b;
                    }

                    if (num == 3){
                        product ^= letter; //product = (letter * 2) ^ (letter*1);
                    }
                }
                final ^= product;
            }
            temp[n+j*4] = convertToByte(final);
            final = 0;
        }
    }
    cpyArray(temp,plaintext);
    return plaintext;
}

int* addRoundKey(int word[16],int key[16]){
    for (int i = 0;i <= 15;i++){
        word[i] = convertToByte(word[i] ^ key[i]);
    }
    return word;
}

int* cpyArray(int src[16],int dest[16]){
    for (int i = 0;i < 16;i++){
        dest[i] = src[i];
    }
}

int convertToByte(int target){ 
    return (target&0xff);
}
//functions for testing 
//print hex is printed in Aes bytes layout
void printHex(int word[16]){
    for (int u = 0;u <= 3;u++){
        for (int v = 0; v <= 15;v += 4){
            printf("%02x ",word[u + v]);
        }
        printf("\n");
    }
    printf("\n"); //easier to see when printHex is called multiple times
}

