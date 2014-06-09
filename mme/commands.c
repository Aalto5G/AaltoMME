/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**@file   commands.c
 * @author Vicent Ferrer
 * @date   March, 2013
 * @brief  Implementation of some server functions to recv commands
 * */

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "commands.h"
#include "MME_engine.h"
#include "logmgr.h"

/*-****************************************************************/
/*static void init_events(){

}*/

int servcommand_init(const int servPort){

    struct sockaddr_in servAddr;
    int servSock;

    /* Create socket for incoming connections */
    servSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (servSock  < 0){
        log_msg(LOG_ERR, errno,"srvcommand_init:server() failed");
        exit(0);
    }
    /* Construct local address structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr=INADDR_ANY;          /* Any incoming interface */
    servAddr.sin_port = htons(servPort);          /* Local port */

    /* Bind to the local address */
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
    err(1,"srvcommand_init: bind() failed");
    }

    return servSock;

}

void cmd_accept(evutil_socket_t listener, short event, void *arg){

    struct t_signal *sig = (struct t_signal *)arg;
    struct comm_tlv cpack;
    size_t len;
    struct sockaddr_in src_addr;
    socklen_t addrlen;
    evutil_socket_t clntSock;
    long long args;
    int status;
    int command;

    /* Set the size of the in-out parameter */
    addrlen = sizeof(struct sockaddr_in);
    /*Read command*/
    addrlen = sizeof(struct sockaddr_in);
    if ((status = recvfrom(listener, &cpack, sizeof(struct comm_tlv), 0, (struct sockaddr *) &src_addr, &addrlen)) < 0 ) {
        log_msg(LOG_ERR, errno, "recvfrom() returned less than 0");
        return;
    }
    log_msg(LOG_DEBUG, 0,"Recvfrom %s:%d command: %d", inet_ntoa(src_addr.sin_addr), src_addr.sin_port, (enum t_comm_name)cpack.t);

    sig->data = NULL;
    switch((enum t_comm_name)cpack.t){
    case engine_signal:
        if (cpack.l == 3 + 8){
            sig->name = cpack.v[0];
            log_msg(LOG_DEBUG, 0, "Recv signal name: %d", cpack.v[0]);
        }
        break;
    case engine_signal_with_args:
        if (cpack.l > 3){
            sig->name = cpack.v[0];
            sig->data = malloc(sizeof(uint64_t));
            sig->freedataFunc = free;
            memcpy(sig->data, &(cpack.imsi), sizeof(uint64_t));

            log_msg(LOG_DEBUG, 0, "Recv signal name: %d, args %llu", cpack.v[0], cpack.imsi);
        }
        break;
    case debug_level:
    /*change debug level*/
        if (cpack.l == 3){
            log_msg(LOG_DEBUG, 0, "Recv debug lvl change req: %d", cpack.v[0]);
            change_logger_lvl(cpack.v[0]);
        }
        break;
    default:
        break;
    }
}
