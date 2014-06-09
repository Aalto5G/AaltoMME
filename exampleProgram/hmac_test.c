/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   hmac_test.c
 * @Author Vicent Ferrer
 * @date   June, 2013
 * @brief  HMAC-SHA1-256 test program
 *
 * Tool to send an engine signal to the mme to create a new user in order to test its load capabilities.
 */

#include <stdio.h>

#include "hmac_sha2.h"


int main (int argc, char **argv){

    const unsigned char key[]={0xe5, 0x64, 0xfd, 0xb6, 0x81, 0x7a}; // e5:64:fd:b6:81:7a
    //const unsigned char key1[]={0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b}; // e5:64:fd:b6:81:7a
    //unsigned int key_size = 6;
    const unsigned char message[]= {0x10, 0x32, 0xf4, 0x77, 0x00, 0x03, 0xe5, 0x64, 0xfd, 0xb6, 0x81, 0x7a, 0x00, 0x06};
    //const unsigned char message1[]= {0x48, 0x69, 0x20, 0x54, 0x68, 0x65, 0x72, 0x65};
    //unsigned int message_len = 14;
    unsigned char mac[32];
    unsigned mac_size = 32, i;

    hmac_sha256(key, sizeof(key), message, sizeof(message), mac, mac_size);

    printf("\nresult:");
    for(i = 0; i<mac_size ; i++){
        if (i%16 == 0){
            printf("\n");
        }
        printf("%.2x ", mac[i]);

    }
    printf("\nlen = %u\n", i);
    return 0;
}
