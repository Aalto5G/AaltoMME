/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   loadtest.c
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME Load Test Tool
 *
 * Tool to send an engine signal to the mme to create a new user in order to test its load capabilities.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "commands.h"
#include "logmgr.h"
#include "signals.h"


int main (int argc, char **argv){
    int sockfd;
    struct sockaddr_in servaddr;
    int i, userssec;
    uint64_t arg2;
    struct comm_tlv msg;
    uint64_t imsi;

    /*Init syslog*/
    init_logger("loadtest", LOG_DEBUG);

    if(argc!=3){
        printf("Parameter number not correct. Usage:\n\tloadtest host users/sec\n");
        exit(1);
    }
    userssec = atoi(argv[2]);

    sockfd=socket(AF_INET, SOCK_DGRAM,0);

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(COMMAND_PORT);

    /*IMSI offset*/
    imsi = 100000000000000ULL;

    for( i = 0; i< 100000000; i++){
        printf("i=%d\n", i);
        if(userssec>0){
            usleep(1000000.0/userssec);
        }
        msg.t=(uint8_t)engine_signal_with_args;
        msg.l=3 + sizeof(uint64_t);
        msg.v[0]=S11_new_user_test;
        memcpy(&(msg.imsi), &arg2, sizeof(uint64_t));
        msg.imsi = imsi + i;
        if(sendto(sockfd, &msg, msg.l ,0, (struct sockaddr *)&servaddr, sizeof(servaddr))<1){
            log_msg(LOG_ERR, errno, "Error sending the command.");
        };
    }

    closelog(); /*Optional*/

    return 0;
}
