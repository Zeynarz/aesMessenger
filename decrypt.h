#include "encrypt.h"
int cipher[16],cipherArr64[64];
int allRoundKeys[11][16];
char decryptedText[65] = "";
//functions
void convertCipher(char* ciphertext,int cipherArr64[64]);
void revSubBytes(int cipher[16]);
void revShiftRows(int cipher[16]);
void revMixColumns(int cipher[16]);
void revAddRoundKey(int cipher[16],int key[16]);
const char* decrypt(char ciphertext[129],int key[16]);

const char* decrypt(char ciphertext[129],int key[16]){
    //need to memset or else old decrypted text will still be here
    memset(decryptedText,'\x00',sizeof(decryptedText));
    //setup round Keys
    cpyArray(key,allRoundKeys[0]);
    for (int round = 1;round <= 10;round++){
        keySchedule(key,round);
        cpyArray(key,allRoundKeys[round]);
    }

    convertCipher(ciphertext,cipherArr64); 
    
    int convertTimes = strlen(ciphertext) / 32;
    for (int i = 0;i < convertTimes;i++){
        for (int j = 0;j < 16;j++)
            cipher[j] = cipherArr64[j + i*16];
        
        //decryption
        revAddRoundKey(cipher,allRoundKeys[10]);
        revShiftRows(cipher);
        revSubBytes(cipher);
        for (int j = 9;j > 0;j--){
            revAddRoundKey(cipher,allRoundKeys[j]);
            revMixColumns(cipher);
            revShiftRows(cipher);
            revSubBytes(cipher);
        }
        revAddRoundKey(cipher,allRoundKeys[0]);

        //check if printable and add into string
        for (int j = 0;j < 16;j++){
            if (cipher[j] == 0)
                break;

            if (cipher[j] < 32 || cipher[j] > 126){
                endwin();
                puts("Decryption Failed");
                puts("Probably Inserted Wrong Key");
                exit(EXIT_FAILURE);
            }
            decryptedText[j + i*16] = cipher[j];
        }
    }
    return decryptedText;
}

void convertCipher(char* ciphertext,int cipherArr64[64]){
    int len = strlen(ciphertext);
    char hex[5];
    if (len % 32 != 0){
        printf("Ciphertext length needs to be divisible by 16\n");
        exit(EXIT_FAILURE);
    }
        
    //convert hex to decimal
    for (int i = 0,j = 0;i < len/2;i++,j += 2){
        sprintf(hex,"%c%c",ciphertext[j],ciphertext[j+1]);
        cipherArr64[i] = strtol(hex,NULL,16);
    }
}

void revSubBytes(int cipher[16]){
    int current,done;
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
    
    //iterate through cipher
    for (int index = 0;index < 16;index++){
        //search through subBox
        current = cipher[index] & 0xff;
        done = false;
        for (int first = 0;first < 16;first++){
            if (done == true)
                    break;
            for (int second = 0;second < 16;second++){
                if (subBox[first][second] == current){
                    cipher[index] = (first * 16) + second;
                    done = true;
                    break;
                }
            }
        }
    }
}

void revShiftRows(int cipher[16]){
    int temp[16];
    cpyArray(cipher,temp);
    for (int row = 1;row <= 3;row++){
        for (int index = row;index <= 3;index++){
            cipher[index] = temp[index+12];
            cipher[index+4] = temp[index];
            cipher[index+8] = temp[index+4];
            cipher[index+12] = temp[index+8];
            cpyArray(cipher,temp);
        }
    }
}

void revMixColumns(int cipher[16]){
    int matrix[16] = {
              0xe,0xb,0xd,0x9,                                  
              0x9,0xe,0xb,0xd,   
              0xd,0x9,0xe,0xb,
              0xb,0xd,0x9,0xe   
        };                                              
    int temp[16];                                       
    int current;                                        
    int letter,num,product;                             
    int final = 0;                                      
    for (int j = 0;j < 4;j++){ 
        for (int n = 0;n < 4;n++){ 
            for (int i = 0;i < 4;i++){ 
                letter = cipher[i+j*4];                                                       
                num = matrix[i+n*4];
                //Galois Field multiplication
                product = gfMultiply(letter,num);
                final ^= product;
            }
            temp[n+j*4] = convertToByte(final);
            final = 0;
        }
    }
    cpyArray(temp,cipher);
}

void revAddRoundKey(int cipher[16],int key[16]){
    for (int i = 0;i < 16;i++){
        cipher[i] =  (cipher[i] ^ key[i]) & 0xff;
    }
}


