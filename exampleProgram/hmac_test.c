/* AaltoMME - Mobility Management Entity for LTE networks
 * Copyright (C) 2013 Vicent Ferrer Guash & Jesus Llorente Santos
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
