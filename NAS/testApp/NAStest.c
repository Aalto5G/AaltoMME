/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and funded by EIT ICT labs.
 */

/**
 * @file   NAStest.c
 * @Author Vicent Ferrer
 * @date   May, 2013
 * @brief
 *
 *
 *  */

#include <stdio.h>
#include <string.h>
#include "NAS.h"

#define MAX_FILE_SIZE 10000

void printfbuffer(uint8_t *buf, uint32_t size){
    uint32_t i;
    printf("The buffer looks like: n %u\n", size);
    for( i = 0; i < size; i=i+2) {
        printf("%.2x%.2x ", buf[i], buf[i+1]);
        if(i%16==14){
            printf("\n");
        }
    }
    printf("\n");
}

int main (int argc, char **argv){
    GenericNASMsg_t msg1, msg2;

    FILE *f;
    unsigned char buffer[MAX_FILE_SIZE], buffer2[MAX_FILE_SIZE];
    unsigned char *pointer;
    uint32_t n;

    if(argc!=2){
        printf("Number of parameters incorrect %d\n\tNAStest filename\n", argc);
        return 1;
    }

    printf("********** NAS Development Test Tool **********\n");
    f = fopen(argv[1], "rb");
    if (f){
        n = fread(buffer, 1, MAX_FILE_SIZE, f);
    }
    else{
        printf("error opening file\n");
        return 1;
    }
    fclose(f);

    /*
    printf("The buffer looks like: n %u\n", 16);
    for( i = 0; i < 16; i++) {
        printf("%#x ", buffer[i]);
        if(i%16==15){
            printf("\n");
        }
    }*/
    printfbuffer(buffer, n);

    dec_NAS(&msg1, buffer, n);
    printf("\n\n");
    dec_NAS(&msg2, ( ((AttachRequest_t*)&(msg1.plain.eMM))->eSM_MessageContainer.v) , ( ((AttachRequest_t*)&(msg1.plain.eMM))->eSM_MessageContainer.l));

    printf("\n\nForge and decode new packed\n");

    /*Forge new EMM AttachRequest packet*/
    pointer = buffer2;

    newNASMsg_EMM(&pointer, EPSMobilityManagementMessages, PlainNAS);
    encaps_EMM(&pointer, AttachRequest);
    /*ePSAttachType*/
    nasIe_v_t1_l(&pointer, 1);
    /*nASKeySetId*/
    nasIe_v_t1_h(&pointer, 7);

    /*ePSMobileId*/
    nasIe_lv_t4(&pointer, (uint8_t*)((AttachRequest_t*)&(msg1.plain.eMM))->ePSMobileId.v, ((AttachRequest_t*)&(msg1.plain.eMM))->ePSMobileId.l);

    /*uENetworkCapability*/
    nasIe_lv_t4(&pointer, ((AttachRequest_t*)&(msg1.plain.eMM))->uENetworkCapability.v, ((AttachRequest_t*)&(msg1.plain.eMM))->uENetworkCapability.l);

    /*eSM_MessageContainer*/
    nasIe_lv_t6(&pointer, ((AttachRequest_t*)&(msg1.plain.eMM))->eSM_MessageContainer.v, ((AttachRequest_t*)&(msg1.plain.eMM))->eSM_MessageContainer.l);

    printfbuffer(buffer2, pointer-buffer2);

    dec_NAS(&msg1, buffer2, pointer-buffer2);

    return 0;
}
