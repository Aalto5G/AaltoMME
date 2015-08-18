/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   ECMSession.h
 * @Author Vicent Ferrer
 * @date   August, 2015
 * @brief  ECM logic
 *
 * This Module implements the ECM session logic, the user associated messages
 * of S1AP
 */

#ifndef S1ASSOC_PRIV_HFILE
#define S1ASSOC_PRIV_HFILE

#include <glib.h>
#include <event2/event.h>
#include <netinet/in.h>

#include "S1Assoc.h"
#include "MME_S1.h"

typedef struct{
	S1                  s1;
	int                 fd;             /**<File Descriptor*/
    struct sockaddr     peerAddr;       /**<Peer IP address, IPv4 or IPv6*/
    socklen_t           socklen;        /**<Peer Socket addr length returned by recvfrom*/
    struct event        *ev;
}S1Assoc_t;

#endif /* S1ASSOC_PRIV_HFILE */
