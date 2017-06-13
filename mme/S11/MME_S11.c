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
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "MME_S11.h"
#include "logmgr.h"
#include "gtp.h"
#include "MME.h"

#include "S11_FSMConfig.h"
#include "S11_User.h"
#include "S11_Peer.h"

typedef struct{
    gpointer    mme;   /**< mme handler*/
    TimerMgr    tm;
    int         fd;    /**< file descriptor of the s11 server*/
    uint32_t    seq : 24 ;
    GHashTable  *users; /**< s11 users by TEID*/
    GHashTable  *peers;
    guint8      restartCounter;
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

static int getLocalRestartCounter(const char *stateDir, const char *ipAddr, guint8 *restartCounter){
    int fd = 0;
    guint8 r=0;
    char filename[512];
    bzero(filename, 512);

    strcat(strcat(strcpy(filename, stateDir), "/recovery-"), ipAddr);
    log_msg(LOG_DEBUG, 0, "Accessing state file %s", filename);

    if(access(filename, R_OK|W_OK)==0){
        fd = open(filename, O_RDWR);
        if(fd == -1){
            log_msg(LOG_ERR, errno, "Couldn't read file %s", filename);
            return -1;
        }
        read(fd, &r, sizeof(guint8));
        log_msg(LOG_DEBUG, 0, "Read from %s restartCounter %d", filename, r);
        r++;
        close(fd);
    }
    fd = open(filename, O_TRUNC | O_RDWR | O_CREAT, 0644);
    if(fd == -1){
        log_msg(LOG_ERR, errno,"Couldn't write/create file %s", filename);
        return -1;
    }
    log_msg(LOG_DEBUG, 0,"Updating restart Counter %d", r);
    write(fd, &r, sizeof(guint8));
    close(fd);

    *restartCounter = r;
    return 0;
}

gpointer s11_init(gpointer mme){
    S11_t *self = g_new0(S11_t, 1);
    struct stat st = {0};

    self->mme = mme;
    self->tm = mme_getTimerMgr(mme);
    self->seq = 0;

    if (stat(mme_getStateDir(self->mme), &st) == -1) {
        mkdir(mme_getStateDir(self->mme), 0755);
    }

    if(getLocalRestartCounter(mme_getStateDir(self->mme),
                              mme_getLocalAddress(self->mme),
                              &self->restartCounter) == -1){
        return NULL;
    }

    s11ConfigureFSM();

    /*Init S11 server*/
    self->fd =init_udp_srv(mme_getLocalAddress(self->mme), GTP2C_PORT);
    if(self->fd == -1){
        log_msg(LOG_ERR, 0, "Error opening the S11 interface. "
                "Check the IP on the configuration");
        s11DestroyFSM();
        return NULL;
    }
    log_msg(LOG_INFO, 0, "Open S11 server on file descriptor %d, port %d",
            self->fd, GTP2C_PORT);

    mme_registerRead(self->mme, self->fd, s11_accept, self);

    self->users = g_hash_table_new_full( g_int_hash,
                                         g_int_equal,
                                         NULL,
                                         (GDestroyNotify)s11u_freeUser);
    self->peers = s11peer_buildTable();

    return self;
}

void s11_free(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;

    s11peer_destroyTable(self->peers);
    g_hash_table_destroy(self->users);
    mme_deregisterRead(self->mme, self->fd);
    close(self->fd);
    s11DestroyFSM();
    g_free(self);
}

void s11_register_fd(gpointer s11_h, int fd, s11_event_cb cb, s11_event_arg arg){
    S11_t *self = (S11_t *) s11_h;
    mme_registerRead(self->mme, fd, cb, arg);
}

void s11_send(gpointer s11_h,
              union gtp_packet *oMsg, guint32 oMsglen,
              struct sockaddr *rAddr, socklen_t rAddrLen,
              GError **err){

    S11_t *self = (S11_t *) s11_h;
    ssize_t ret = 0;
    ret = sendto(self->fd, oMsg, oMsglen, 0, rAddr, rAddrLen);
    if(ret<0){
        /* *err = g_error_new(); */
        log_errpack(LOG_ERR, errno, (struct sockaddr_in *)rAddr,
                    oMsg, oMsglen,
                    "Sendto(fd=%d, msg=%lx, len=%d) failed",
                    self->fd, (unsigned long) oMsg, oMsglen);
        g_error("Error sendto");
    }
}

const guint8 getRestartCounter(gpointer s11_h){
    S11_t *self = (S11_t *) s11_h;
    return self->restartCounter;
}

gboolean S11_isFirstSession(gpointer  s11_h,
                            const struct sockaddr *rAddr,
                            const socklen_t rAddrLen){
    S11_t *self = (S11_t *)s11_h;
    Peer_t *p = NULL;
    if(s11peer_isFirstSession(self->peers, rAddr, rAddrLen, &p)){
        p->s11 = self;
        p->tm = self->tm;
        s11peer_track(p);
        return TRUE;
    }
    return FALSE;
}

void S11_unrefSession(gpointer  s11_h,
                      const struct sockaddr *rAddr,
                      const socklen_t rAddrLen){
    S11_t *self = (S11_t *)s11_h;

    Peer_t * p = s11peer_get(self->peers, rAddr, rAddrLen);
    if(!p){
        log_msg(LOG_ERR, 0,"S11 Peer was not tracked");
    }
    p->num_sessions--;

    if(p->num_sessions==0){
        log_msg(LOG_INFO, 0,"S11 Peer last session, untracking");
        s11peer_untrack(p);
        g_hash_table_remove(self->peers, p);
    }
}

void S11_checkPeerRestart(gpointer  s11_h,
                          const struct sockaddr *rAddr,
                          const socklen_t rAddrLen,
                          guint8 restartCounter,
                          gpointer ongoingUser){
    S11_t *self = (S11_t *)s11_h;
    char addrStr[INET6_ADDRSTRLEN];

    if(s11peer_hasRestarted(self->peers, rAddr, rAddrLen, restartCounter)){
        log_msg(LOG_WARNING, 0, "Peer restart Detected %s",
                inet_ntop(rAddr->sa_family,
                          &((struct sockaddr_in*)rAddr)->sin_addr,
                          addrStr,
                          rAddrLen));
        /* TODO Remove contexts affected*/
    }
}

void S11_paging(gpointer s11_h, gpointer emm){
    S11_t *self = (S11_t *)s11_h;
    mme_paging(self->mme, emm);
}

static void processEchoReq(S11_t *self, struct t_message *msg){
    union gtp_packet   oMsg = {0};
    uint32_t           oMsglen = 0;
    guint              ienum = 0;
    union gtpie_member *echo_ie[GTPIE_SIZE];
    union gtpie_member ie[3] = {0};
    guint8             value[GTP2IE_MAX] = {0};
    guint16            vsize = 0;
    char addrStr[INET6_ADDRSTRLEN];
    GError *err = NULL;

    gtp2ie_decap(echo_ie, &msg->packet, msg->length);
    gtp2ie_gettliv(echo_ie,  GTPV2C_IE_RECOVERY, 0, value, &vsize);
    log_msg(LOG_INFO, 0, "Received ECHO REQ from %s, recovery %u",
            inet_ntop(msg->peer.sa_family,
                      &((struct sockaddr_in*)&msg->peer)->sin_addr,
                      addrStr,
                      msg->peerlen),
            value[0]);

    S11_checkPeerRestart(self, &msg->peer, msg->peerlen, value[0], NULL);

    /* Reply */
    oMsglen = get_default_gtp(2, GTP2_ECHO_RSP, &oMsg);
    oMsg.gtp2s.h.seq = msg->packet.gtp.gtp2s.h.seq;

    /* Recovery IE*/
    ie[ienum].tliv.i=0;
    ie[ienum].tliv.l=hton16(1);
    ie[ienum].tliv.t=GTPV2C_IE_RECOVERY;
    ie[ienum].tliv.v[0]=self->restartCounter;
    ienum++;
    gtp2ie_encaps(ie, ienum, &oMsg, &oMsglen);

    /* Send */
    s11_send(self, &oMsg, oMsglen,
              &(msg->peer), msg->peerlen, &err);
    if(err != NULL){
        log_msg(LOG_ERR, 0, "s11_send error");
    }
}


void s11_accept(evutil_socket_t listener, short event, void *arg){

    uint32_t teid;
    struct t_message *msg;

    struct sockaddr_in *ipv4;

    S11_t *self = (S11_t *) arg;
    gpointer session;    /* S11_user_t * u; */
    const char str[INET6_ADDRSTRLEN];

    log_msg(LOG_DEBUG, 0, "Enter");

    msg = newMsg();
    msg->packet.gtp.flags=0x0;
    if (gtp2_recv(listener, &(msg->packet.gtp), &(msg->length),
                  &msg->peer, &msg->peerlen) != 0 ){
        log_errpack(LOG_ERR, errno, (struct sockaddr_in *)&(msg->peer),
                    &(msg->packet.gtp), msg->length,
                    "gtp2_recv(fd=%d, msg=%lx, len=%d) failed",
                    listener, (unsigned long) &(msg->packet.gtp), msg->length);
    }

    if(msg->packet.gtp.gtp2s.h.type == GTP2_ECHO_REQ ){
        processEchoReq(self, msg);
    }else if(msg->packet.gtp.gtp2s.h.type == GTP2_ECHO_RSP){
        s11peer_processEchoRsp(self->peers,
                               &msg->peer, msg->peerlen,
                               &msg->packet.gtp, msg->length);
    }else if(msg->packet.gtp.gtp2s.h.type<4){
        /* TODO @Vicent:
           Manage echo request, echo response or version not suported*/
        log_msg(LOG_INFO, 0, "S11 recv echo request,"
                " echo response or version not suported msg");
        print_packet(&(msg->packet), msg->length);
    }else{
        teid = ntoh32(msg->packet.gtp.gtp2l.h.tei);

        if (! g_hash_table_lookup_extended(self->users, &teid, NULL, &session)){
            log_errpack(LOG_INFO, 0, (struct sockaddr_in*)&(msg->peer),
                        &(msg->packet), msg->length,
                        "S11 received packet with unknown TEID (%#X),"
                        " ignoring packet", &teid);
        }else{
            processMsg(session, msg);
        }
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

void S11_detach(gpointer s11_user, void(*cb)(gpointer), gpointer args){
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
