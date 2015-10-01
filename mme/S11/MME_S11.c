/* This application was initially developed as a Final Project by
 *     Vicent Ferrer Guasch (vicent.ferrerguasch@aalto.fi)
 * under the supervision of,
 *     Jukka Manner (jukka.manner@aalto.fi)
 *     Jose Costa-Requena (jose.costa@aalto.fi)
 * in AALTO University and partially funded by EIT ICT labs.
 */

/**
 * @file   MME_S11.c
 * @Author Vicent Ferrer
 * @date   March, 2013
 * @brief  MME S11 interface protocol state machine.
 *
 * This module implements the S11 interface state machine. The states are separated on STATE_* and TASK_MME_S11___*
 * The firsts ones implements the flow control and the second ones the processing
 */

//#include "MME_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "signals.h"

#include "MME_S11.h"
#include "logmgr.h"
#include "nodemgr.h"
#include "gtp.h"
#include "MME.h"

#include "S11_FSMConfig.h"
#include "S11_User.h"

typedef struct{
    gpointer    mme;   /**< mme handler*/
    int         fd;    /**< file descriptor of the s11 server*/
    uint32_t    seq : 24 ;
    GHashTable  *users; /**< s11 users by TEID*/
}S11_t;

void s11_accept(evutil_socket_t listener, short event, void *arg);

/* ======================================================================*/

static gpointer s11_newSession(S11_t *s11, EMMCtx emm, EPS_Session s){
    gpointer u = s11u_newUser(s11, emm, s);
    g_hash_table_insert(s11->users, s11u_getTEIDp(u), u);
    return u;
}

void s11_deleteSession(gpointer s11_h, gpointer u){
    S11_t *self = (S11_t *) s11_h;
    g_hash_table_remove(self->users, s11u_getTEIDp(u));
}


gpointer s11_init(gpointer mme){
    S11_t *self = g_new0(S11_t, 1);

    self->mme = mme;
    self->seq = 0;

    s11ConfigureFSM();

    /*Init S11 server*/
    self->fd =init_udp_srv(mme_getLocalAddress(self->mme), GTP2C_PORT);
    log_msg(LOG_INFO, 0, "Open S11 server on file descriptor %d, port %d",
            self->fd, GTP2C_PORT);

    mme_registerRead(self->mme, self->fd, s11_accept, self);

    self->users = g_hash_table_new_full( g_int_hash,
                                         g_int_equal,
                                         NULL,
                                         (GDestroyNotify)s11u_freeUser);
    return self;
}

void s11_free(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;

    g_hash_table_destroy(self->users);
    mme_deregisterRead(self->mme, self->fd);
    close(self->fd);
    s11DestroyFSM();
    g_free(self);
}

const int s11_fg(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;
    return self->fd;
}


void s11_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;
    struct t_message *msg;

    struct sockaddr peer;
    struct sockaddr_in *ipv4;
    socklen_t peerlen;

    S11_t *self = (S11_t *) arg;
    gpointer session;    /* S11_user_t * u; */
    const char str[INET6_ADDRSTRLEN];

    log_msg(LOG_DEBUG, 0, "Enter");

    msg = newMsg();
    msg->packet.gtp.flags=0x0;
    if (gtp2_recv(listener, &(msg->packet.gtp), &(msg->length),
                  &peer, &peerlen) != 0 ){
        log_errpack(LOG_ERR, errno, (struct sockaddr_in *)&(peer),
                    &(msg->packet.gtp), msg->length,
                    "gtp2_recv(fd=%d, msg=%lx, len=%d) failed",
                    listener, (unsigned long) &(msg->packet.gtp), msg->length);
    }

    switch(peer.sa_family){
    case AF_INET:
        ipv4 = (struct sockaddr_in*) &peer;
        inet_ntop(AF_INET, &(ipv4->sin_addr), msg->srcAddr, INET_ADDRSTRLEN);
    /* case AF_INET6: */
    /*     inet_ntop(AF_INET6, &(peer.sin6_addr), str, INET6_ADDRSTRLEN); */
    }

    if(msg->packet.gtp.gtp2s.h.type<4){
        /* TODO @Vicent:
           Manage echo request, echo response or version not suported*/
        log_msg(LOG_INFO, 0, "S11 recv echo request,"
                " echo response or version not suported msg");
        print_packet(&(msg->packet), msg->length);
        return;
    }

    teid = ntoh32(msg->packet.gtp.gtp2l.h.tei);

    if (! g_hash_table_lookup_extended(self->users, &teid, NULL, &session)){
        log_errpack(LOG_INFO, 0, (struct sockaddr_in*)&(peer),
                    &(msg->packet), msg->length,
                    "S11 received packet with unknown TEID (%#X),"
                    " ignoring packet", &teid);
        return;
    }

    if (s11u_hasPendingResp( session)){
        log_msg(LOG_DEBUG, 0, "Received pending S11 reply");
        processMsg(session, msg);
    }
    else{
        log_msg(LOG_DEBUG, 0, "Received new S11 request");
        processMsg(session, msg);
    }

    freeMsg(msg);
}

const unsigned int getNextSeq(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;
    return self->seq++;
}

const char *s11_getLocalAddress(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;
    return mme_getLocalAddress(self->mme);
}

gpointer S11_newUserAttach(gpointer s11_h, EMMCtx emm, EPS_Session s,
                        void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter S11_newUserAttach()");

    S11_t *self = (S11_t *) s11_h;

    /* New MME TEID for the new user*/
    gpointer u = s11_newSession(self, emm, s);

    attach(u, cb, args);
    return u;
}


void S11_Attach_ModifyBearerReq(gpointer s11_user, void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter");
    modBearer(s11_user, cb, args);
}

void S11_dettach(gpointer s11_user, void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter");
    detach(s11_user, cb, args);
}

void S11_ReleaseAccessBearers(gpointer s11_user, void(*cb)(gpointer), gpointer args){
    log_msg(LOG_DEBUG, 0, "enter");
    /* For Future use, requires S-GW compatibility*/
    releaseAccess(s11_user, cb, args);
}

/* void S11_CreateIndirectDataForwardingTunnel(gpointer s11_user, void(*cb)(gpointer), gpointer args){ */
/*     log_msg(LOG_DEBUG, 0, "enter"); */

/*     u->state = STATE_CreateIndirectDataForwardingTunnel; */
/*      u->state(u, S11_createIndirectDataForwardingTunnel); */

/*      //cb(args); */
/* } */
